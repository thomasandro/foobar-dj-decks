#pragma once
#include "stem_cache.h"
#include <atomic>
#include <thread>
#include <functional>
#include <filesystem>
namespace dj_stems {
inline std::string utf8(const std::wstring& w){
    int n=WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),nullptr,0,nullptr,nullptr);std::string s(n,'\0');WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),s.data(),n,nullptr,nullptr);return s;
}
inline std::wstring wide(const std::string& s){
    int n=MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0);std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),w.data(),n);return w;
}
struct Job {
    std::string sourceKey;std::wstring work;
    std::atomic<bool> cancel{false};std::atomic<int> progress{0},state{0}; // 1 export, 2 model, 3 ready, 4 error, 5 cancelled
    std::thread thread;std::wstring error;
    ~Job(){stop();}
    void stop(){cancel=true;if(thread.joinable())thread.join();}
    void start(const std::string& key,std::function<void(Job&,const std::wstring&)> exportAudio,std::wstring runtimeRoot=L""){
        sourceKey=key;work=cacheRoot()+L"\\job-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
        std::filesystem::create_directories(work);state=1;
        thread=std::thread([this,exportAudio,runtimeRoot]{
            SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
            HANDLE process=nullptr,job=nullptr,log=INVALID_HANDLE_VALUE,input=INVALID_HANDLE_VALUE;
            int terminalState=4;
            try {
                exportAudio(*this,work+L"\\input.wav");
                if(cancel)throw std::runtime_error("Cancelled");
                picojson::object request;request["protocol"]=picojson::value(1.0);request["key"]=picojson::value(sourceKey);request["output"]=picojson::value(utf8(folder(sourceKey)));
                auto requestPath=work+L"\\job.json";
                {std::ofstream out(requestPath,std::ios::binary);auto text=picojson::value(request).serialize();out<<text;if(!out)throw std::runtime_error("Cannot write stem job");}
                auto engineRoot=runtimeRoot.empty()?localRoot()+L"\\stems-engine":runtimeRoot;
                auto python=engineRoot+L"\\venv\\Scripts\\python.exe";
                auto command=L"\""+python+L"\" \""+engineRoot+L"\\stems_worker.py\" --job \""+requestPath+L"\"";
                SECURITY_ATTRIBUTES sa={sizeof(sa),nullptr,TRUE};
                log=CreateFileW((work+L"\\worker.log").c_str(),GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
                input=CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,0,nullptr);
                if(log==INVALID_HANDLE_VALUE||input==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot prepare stem worker handles");
                STARTUPINFOEXW startup={};startup.StartupInfo.cb=sizeof(startup);startup.StartupInfo.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;startup.StartupInfo.wShowWindow=SW_HIDE;
                startup.StartupInfo.hStdInput=input;startup.StartupInfo.hStdOutput=log;startup.StartupInfo.hStdError=log;PROCESS_INFORMATION pi={};
                job=CreateJobObjectW(nullptr,nullptr);if(!job)throw std::runtime_error("Cannot create stem process job");
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)))throw std::runtime_error("Cannot protect stem process lifetime");
                SIZE_T attrSize=0;InitializeProcThreadAttributeList(nullptr,1,0,&attrSize);std::vector<unsigned char> attributes(attrSize);
                startup.lpAttributeList=(LPPROC_THREAD_ATTRIBUTE_LIST)attributes.data();
                if(!InitializeProcThreadAttributeList(startup.lpAttributeList,1,0,&attrSize))throw std::runtime_error("Cannot initialize stem process attributes");
                HANDLE inherited[]={log,input};
                if(!UpdateProcThreadAttribute(startup.lpAttributeList,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,inherited,sizeof(inherited),nullptr,nullptr)){DeleteProcThreadAttributeList(startup.lpAttributeList);throw std::runtime_error("Cannot restrict worker handle inheritance");}
                bool created=CreateProcessW(python.c_str(),command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW|BELOW_NORMAL_PRIORITY_CLASS|CREATE_SUSPENDED|EXTENDED_STARTUPINFO_PRESENT,nullptr,engineRoot.c_str(),&startup.StartupInfo,&pi)!=FALSE;
                DeleteProcThreadAttributeList(startup.lpAttributeList);
                if(!created)throw std::runtime_error("Stem engine unavailable. Run stems/install-stems.ps1 first.");
                process=pi.hProcess;
                if(!AssignProcessToJobObject(job,process)){TerminateProcess(process,1);CloseHandle(pi.hThread);throw std::runtime_error("Cannot attach stem worker to lifetime job");}
                ResumeThread(pi.hThread);CloseHandle(pi.hThread);state=2;progress=20;
                DWORD wait;
                while((wait=WaitForSingleObject(process,250))==WAIT_TIMEOUT){
                    if(cancel){TerminateJobObject(job,1);WaitForSingleObject(process,10000);throw std::runtime_error("Cancelled");}
                    auto status=readJson(work+L"\\status.json",8192);auto& n=status.get("progress");
                    if(n.is<double>()&&std::isfinite(n.get<double>()))progress=20+(int)((std::max)(0.0,(std::min)(100.0,n.get<double>()))*.79);
                }
                DWORD code=1;if(wait!=WAIT_OBJECT_0||!GetExitCodeProcess(process,&code))throw std::runtime_error("Stem worker wait failed");
                if(code){auto status=readJson(work+L"\\status.json",8192);auto& m=status.get("message");throw std::runtime_error(m.is<std::string>()?m.get<std::string>():"Stem worker failed");}
                Reader verified(folder(sourceKey),sourceKey);progress=100;terminalState=3;
            }catch(const std::exception& e){error=wide(e.what());terminalState=cancel?5:4;}
            if(job)CloseHandle(job);if(process)CloseHandle(process);
            if(log!=INVALID_HANDLE_VALUE)CloseHandle(log);if(input!=INVALID_HANDLE_VALUE)CloseHandle(input);
            // Only these files are owned staging data. Never recursively remove cache/user paths.
            // Worker-tree handle release can lag the main process signal on Windows.
            // Retry only our fixed staging filenames, with a bounded wait.
            for(int attempt=0;attempt<10;++attempt){
                for(auto name:{L"input.wav",L"job.json",L"status.json",L"status.json.tmp",L"worker.log"})DeleteFileW((work+L"\\"+name).c_str());
                for(auto name:names)DeleteFileW((work+L"\\result\\"+name).c_str());
                DeleteFileW((work+L"\\result\\manifest.json").c_str());DeleteFileW((work+L"\\result\\manifest.json.tmp").c_str());
                RemoveDirectoryW((work+L"\\result").c_str());
                if(RemoveDirectoryW(work.c_str()) || GetFileAttributesW(work.c_str())==INVALID_FILE_ATTRIBUTES)break;
                Sleep(50);
            }
            // Publish completion only after resources/staging are released: the UI
            // must not join a still-cleaning worker from its completion handler.
            state=terminalState;
        });
    }
};
}
