#define NOMINMAX
#define UNICODE
#define _UNICODE
#include "../sdk/foobar2000/foo_dj/stem_cache_manager.h"
#include <cassert>
#include <iostream>
namespace fs=std::filesystem;
void write(const fs::path& p){std::ofstream f(p,std::ios::binary);f<<"1234";}
int wmain(int argc,wchar_t** argv){
    if(argc==2){SetEnvironmentVariableW(L"LOCALAPPDATA",argv[1]);assert(dj_stems::inspectCache(true).errors==1);std::cout<<"Junction cache root/parent refused\n";return 0;}
    auto base=fs::absolute(fs::path("build/tests")/(L"cache-manager-"+std::to_wstring(GetTickCount64())));
    fs::create_directories(base);SetEnvironmentVariableW(L"LOCALAPPDATA",base.c_str());
    using namespace dj_stems;fs::create_directories(cacheRoot());
    auto add=[](wchar_t c){std::wstring key(64,c);auto p=fs::path(cacheRoot())/key;fs::create_directory(p);for(auto n:names)write(p/n);write(p/L"manifest.json");return key;};
    auto a=add(L'a'),b=add(L'b'),c=add(L'c'),d=add(L'd'),e=add(L'e'),f=add(L'f');
    auto root=fs::path(cacheRoot());write(root/b/L"unknown.txt");
    fs::create_directory(root/L"job-do-not-remove");write(root/L"job-do-not-remove/input.wav");
    fs::create_directory(base/L"DJDecks/stems-engine");write(base/L"DJDecks/stems-engine/model.th");write(base/L"source.flac");
    CacheHandle active;active.h=CreateFileW((root/c/L"drums.f32").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);assert(active.h!=INVALID_HANDLE_VALUE);
    CacheHandle worker;worker.h=CreateFileW((root/(d+L".lock")).c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,0,nullptr);OVERLAPPED ov={};assert(LockFileEx(worker.h,LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,0,1,0,&ov));
    assert(SetFileAttributesW((root/e/L"vocals.f32").c_str(),FILE_ATTRIBUTE_READONLY));
    auto before=inspectCache(false);assert(before.entries==6&&before.bytes==124&&before.errors==0);
    auto cleared=inspectCache(true,{f});assert(cleared.removed==1&&cleared.removedBytes==20&&cleared.kept==5&&cleared.errors==0);
    assert(!fs::exists(root/a));for(auto key:{b,c,d,e,f})assert(fs::exists(root/key/L"manifest.json"));
    assert(fs::exists(root/L"job-do-not-remove/input.wav")&&fs::exists(base/L"DJDecks/stems-engine/model.th")&&fs::exists(base/L"source.flac"));
    CloseHandle(active.h);active.h=INVALID_HANDLE_VALUE;CloseHandle(worker.h);worker.h=INVALID_HANDLE_VALUE;assert(SetFileAttributesW((root/e/L"vocals.f32").c_str(),FILE_ATTRIBUTE_NORMAL));
    auto again=inspectCache(true);assert(again.removed==4&&again.kept==1&&again.errors==0);assert(inspectCache(false).entries==1);
    // A linked cache child must never cause traversal/deletion of its destination.
    auto outside=base/L"outside";fs::create_directory(outside);write(outside/L"manifest.json");auto link=root/std::wstring(64,L'9');
    bool linkTest=CreateSymbolicLinkW(link.c_str(),outside.c_str(),SYMBOLIC_LINK_FLAG_DIRECTORY|SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)!=FALSE;
    if(linkTest){auto r=inspectCache(true);assert(r.errors>=1&&fs::exists(outside/L"manifest.json"));}
    // A reparse-point root must also be refused without enumerating its target.
    auto linkedBase=base/L"linked-parent";fs::create_directories(linkedBase/L"DJDecks/stems-cache");
    if(CreateSymbolicLinkW((linkedBase/L"DJDecks/stems-cache/v1").c_str(),outside.c_str(),SYMBOLIC_LINK_FLAG_DIRECTORY|SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)){
        SetEnvironmentVariableW(L"LOCALAPPDATA",linkedBase.c_str());assert(inspectCache(true).errors==1&&fs::exists(outside/L"manifest.json"));
    }else linkTest=false;
    std::cout<<"Cache manager: size, protected keys, open readers, worker byte locks, unknown files, read-only preflight, repeat clear and source/model/job preservation passed; reparse tests "<<(linkTest?"passed":"partly skipped (symlink privilege)")<<"\n";
}
