#define NOMINMAX
#include "../sdk/foobar2000/foo_dj/stem_job.h"
#include <cassert>
#include <iostream>
int wmain(int argc,wchar_t** argv){
    assert(argc==2);wchar_t root[32768];GetFullPathNameW(L"build\\tests\\stem-worker-user",32768,root,nullptr);
    SetEnvironmentVariableW(L"LOCALAPPDATA",root);
    std::string key(64,'c');auto stamp=std::to_string(GetTickCount64());key.replace(0,stamp.size(),stamp);
    dj_stems::Job job;job.start(key,[](dj_stems::Job& j,const std::wstring& input){
        dj_stems::WaveWriter w(input,44100);std::vector<float> samples;
        for(int n=0;n<44100*3;++n){samples.push_back((float)(.1*sin(n*2*3.141592653589793*440/44100)));samples.push_back((float)(.1*sin(n*2*3.141592653589793*220/44100)));}
        w.append(samples.data(),44100*3,2);w.finish();j.progress=19;
    },argv[1]);
    for(int n=0;n<1200&&job.state<3;++n)Sleep(100);
    if(job.state!=3){std::wcerr<<job.error<<L"\n";return 1;}job.thread.join();
    dj_stems::Reader reader(dj_stems::folder(key),key);assert(reader.frames==44100*3);
    std::vector<double> all;reader.read(all,(size_t)reader.frames);double worst=0;
    for(size_t n=0;n<reader.frames;++n){worst=(std::max)(worst,std::abs(all[n*2]-.1*sin(n*2*3.141592653589793*440/44100)));worst=(std::max)(worst,std::abs(all[n*2+1]-.1*sin(n*2*3.141592653589793*220/44100)));}
    assert(worst<1e-6);
    // Repeated job is an actual worker cache hit; it must not replace the cache.
    auto before=std::filesystem::last_write_time(dj_stems::folder(key)+L"\\manifest.json");
    dj_stems::Job hit;hit.start(key,[](dj_stems::Job&,const std::wstring& input){dj_stems::WaveWriter w(input,44100);float a[2]={0,0};w.append(a,1,2);w.finish();},argv[1]);
    for(int n=0;n<600&&hit.state<3;++n)Sleep(100);if(hit.state!=3){std::wcerr<<hit.error<<L"\n";return 1;}hit.thread.join();assert(before==std::filesystem::last_write_time(dj_stems::folder(key)+L"\\manifest.json"));
    dj_stems::Job cancelled;cancelled.start(std::string(64,'d'),[](dj_stems::Job& j,const std::wstring&){while(!j.cancel)Sleep(10);throw std::runtime_error("cancelled");},argv[1]);Sleep(30);cancelled.stop();assert(cancelled.state==5);assert(!std::filesystem::exists(cancelled.work));
    std::string cancelKey=key;cancelKey.back()='f';dj_stems::Job running;
    running.start(cancelKey,[](dj_stems::Job&,const std::wstring& input){dj_stems::WaveWriter w(input,44100);std::vector<float> audio(44100*6,.01f);w.append(audio.data(),44100*3,2);w.finish();},argv[1]);
    for(int n=0;n<100&&running.state==1;++n)Sleep(20);assert(running.state==2);Sleep(200);running.stop();
    if(running.state!=5||std::filesystem::exists(running.work)||std::filesystem::exists(dj_stems::folder(cancelKey))){std::wcerr<<L"Cancel state="<<running.state.load()<<L" work remains="<<std::filesystem::exists(running.work)<<L" error="<<running.error<<L"\n";return 1;}
    std::cout<<"Real native export -> Python Demucs -> atomic cache -> native reader passed; max all-stem reconstruction error="<<worst<<"; cache hit and cancellation passed\n";
}
