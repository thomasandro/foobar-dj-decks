#define NOMINMAX
#include <windows.h>
#include <string>
#include <cassert>
#include <iostream>
#include <limits>
#include "../sdk/foobar2000/foo_dj/loop_storage.h"
static std::wstring fixture;
static std::wstring SessionFile(const char* name){return fixture+L"\\"+std::wstring(name,name+strlen(name));}
#include "../build/tests/loop-storage-io.inc"
int main(){
 wchar_t temp[MAX_PATH];assert(GetTempPathW(MAX_PATH,temp));fixture=std::wstring(temp)+L"dj-loop-storage-"+std::to_wstring(GetCurrentProcessId());assert(CreateDirectoryW(fixture.c_str(),nullptr));
 using namespace dj_loop;picojson::object data;
 const std::string key="[\"file:///music/été.opus\",0]",sub="[\"file:///music/été.opus\",1]";
 remember(data,key,{12.345,26.789,true});remember(data,sub,{1,-1,false});
 assert(WriteSessionFile(picojson::value(data).serialize(),"loops.json"));picojson::value saved;assert(picojson::parse(saved,ReadSessionFile("loops.json")).empty());
 data=saved.get<picojson::object>();auto region=restore(data,key,0,100);assert(region.a==12.345&&region.b==26.789&&!region.enabled);
 assert(restore(data,sub,0,100).a==1&&restore(data,sub,0,100).b<0);assert(restore(data,"other",0,100).a<0);
 assert(restore(data,key,13,100).a<0);assert(restore(data,key,0,20).a==12.345&&restore(data,key,0,20).b<0);
 remember(data,key,{15,30,false});assert(WriteSessionFile(picojson::value(data).serialize(),"loops.json"));
 assert(picojson::parse(saved,ReadSessionFile("loops.json")).empty());assert(restore(saved.get<picojson::object>(),key,0,100).a==15);
 remember(data,key,{});assert(WriteSessionFile(picojson::value(data).serialize(),"loops.json"));assert(picojson::parse(saved,ReadSessionFile("loops.json")).empty());assert(saved.get<picojson::object>().count(key)==0&&saved.get<picojson::object>().count(sub)==1);
 for(auto bad:{picojson::value(),picojson::value("bad"),picojson::value(picojson::object{{"a",picojson::value(-1.)}}),picojson::value(picojson::object{{"a",picojson::value(1.)},{"b",picojson::value(.5)}})}){
  data[key]=bad;auto result=restore(data,key,0,100);assert(!result.enabled&&result.b<0);
 }
 auto previous=ReadSessionFile("loops.json");auto held=CreateFileW(SessionFile("loops.json").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);assert(held!=INVALID_HANDLE_VALUE);assert(!WriteSessionFile("replacement","loops.json"));CloseHandle(held);assert(ReadSessionFile("loops.json")==previous);
 assert(!ReadSessionFile("missing.json").size());assert(WriteSessionFile("broken json","loops.json"));assert(!picojson::parse(saved,ReadSessionFile("loops.json")).empty());
 assert(DeleteFileW(SessionFile("loops.json").c_str()));assert(RemoveDirectoryW(fixture.c_str()));
 std::cout<<"Actual atomic file writer/reader + loop storage: restart round-trip, Unicode/subsongs, partial A, overwrite, deletion, trimmed bounds, malformed/missing data and disabled activation passed\n";
}
