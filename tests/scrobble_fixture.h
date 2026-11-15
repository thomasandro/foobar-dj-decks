#include "../sdk/foobar2000/foo_dj/listening_policy.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <vector>
using ULONGLONG=unsigned long long;static ULONGLONG ticks=1000;inline ULONGLONG GetTickCount64(){return ticks;}
static std::atomic<bool> g_scrobble{true},g_scrobbleReady{false};
struct Text:std::string{using std::string::string;bool is_empty()const{return empty();}};
struct file_info_impl{Text artist="artist",title="title";};
struct Handle{file_info_impl info;bool known=true;bool get_info(file_info_impl& out){out=info;return known;}};
template<class T>struct Ptr:std::shared_ptr<T>{using std::shared_ptr<T>::shared_ptr;Ptr()=default;Ptr(std::shared_ptr<T> p):std::shared_ptr<T>(p){}bool is_valid()const{return bool(*this);}};
using metadb_handle_ptr=Ptr<Handle>;
struct metadb{std::map<std::string,metadb_handle_ptr> items;static metadb* get(){static metadb m;return &m;}metadb_handle_ptr handle_create(const char* path,unsigned){auto& h=items[path];if(!h)h=std::make_shared<Handle>();return h;}};
struct playback_control{metadb_handle_ptr normal;static playback_control* get(){static playback_control p;return &p;}void get_now_playing(metadb_handle_ptr& p){p=normal;}};
namespace foo_scrobble {
struct Track{Text Artist,Title;};
struct ScrobbleService{using ptr=Ptr<ScrobbleService>;int calls=0;bool fail=false;void ScrobbleAsync(Track){++calls;if(fail)throw std::runtime_error("mock service failure");}};
}
namespace dj_scrobble {
static bool enabled=true,available=true;static auto backend=std::make_shared<foo_scrobble::ScrobbleService>();
inline foo_scrobble::ScrobbleService::ptr service(){return available?foo_scrobble::ScrobbleService::ptr(backend):foo_scrobble::ScrobbleService::ptr{};}
inline bool hostEnabled(){return enabled;}
inline foo_scrobble::Track track(const file_info_impl& info,double,long long){return {info.artist,info.title};}
}
namespace console{inline void print(const char*){}}
class CDjPanel{
public:
    ULONGLONG m_lastScrobbleTick=0;std::atomic<bool> m_scrobbleAllowed[2]={{false},{false}};
    struct Deck{dj_listening::Clock listening;double length=100;};Deck m_deck[2];
    std::string m_savedPath[2]={"a","b"};unsigned m_savedSubsong[2]={};std::mutex m_lock;
    void ScrobbleTick();
    void qualify(int i){m_deck[i].listening.consume(48000*50,48000,1,true,1000);}
    void tick(){ticks+=1000;ScrobbleTick();}
};
