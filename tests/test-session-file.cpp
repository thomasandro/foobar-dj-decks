#define NOMINMAX
#include "../sdk/foobar2000/foo_dj/session_bundle_io.h"
#include <cassert>
#include <iostream>
int main(){wchar_t temp[MAX_PATH];assert(GetTempPathW(MAX_PATH,temp));std::wstring folder=std::wstring(temp)+L"dj-bundle-"+std::to_wstring(GetCurrentProcessId());assert(CreateDirectoryW(folder.c_str(),nullptr));auto path=folder+L"\\été.dj-decks.json";
 dj_session::Bundle b;b.lists[0].items={{"file:///été.opus",1},{"file:///été.opus",1}};auto first=dj_session::serialize(b);assert(dj_session::write(path,first));auto restored=dj_session::parse(dj_session::read(path));assert(restored.lists[0].items.size()==2&&restored.lists[0].items[0].subsong==1);
 HANDLE held=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);assert(held!=INVALID_HANDLE_VALUE);b.lists[1].items={{"second",0}};auto second=dj_session::serialize(b);assert(!dj_session::write(path,second));CloseHandle(held);assert(dj_session::read(path)==first);assert(dj_session::write(path,second)&&dj_session::read(path)==second);
 bool invalid=false;try{dj_session::parse("broken");}catch(...){invalid=true;}assert(invalid);assert(!dj_session::write(L"",first));assert(DeleteFileW(path.c_str()));assert(RemoveDirectoryW(folder.c_str()));std::cout<<"Session file IO: Unicode file path, read/replace round-trip, locked-target rollback, temporary-file cleanup and malformed JSON passed (generated temp files only)\n";}
