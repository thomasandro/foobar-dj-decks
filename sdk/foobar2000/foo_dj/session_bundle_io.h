#pragma once
#include "session_bundle.h"
#include <windows.h>
#include <string>
namespace dj_session {
inline std::string read(const std::wstring& path){HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot read session file");LARGE_INTEGER size={};bool ok=GetFileSizeEx(file,&size)&&size.QuadPart>0&&size.QuadPart<=maxBytes;std::string text;if(ok){text.resize((size_t)size.QuadPart);DWORD got=0;ok=ReadFile(file,text.data(),(DWORD)text.size(),&got,nullptr)&&got==text.size();}CloseHandle(file);require(ok,"Invalid session file size or read error");return text;}
inline bool write(const std::wstring& path,const std::string& text){
 if(path.empty()||text.empty()||text.size()>maxBytes)return false;
 std::wstring temp;HANDLE file=INVALID_HANDLE_VALUE;for(int n=0;n<5;++n){temp=path+L"."+std::to_wstring(GetCurrentProcessId())+L"."+std::to_wstring(GetTickCount64())+L"."+std::to_wstring(n)+L".tmp";file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file!=INVALID_HANDLE_VALUE)break;if(GetLastError()!=ERROR_FILE_EXISTS)return false;}
 if(file==INVALID_HANDLE_VALUE)return false;DWORD written=0;bool ok=WriteFile(file,text.data(),(DWORD)text.size(),&written,nullptr)&&written==text.size()&&FlushFileBuffers(file);CloseHandle(file);
 if(ok)ok=MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;if(!ok)DeleteFileW(temp.c_str());return ok;
}
}
