#include "../sdk/foobar2000/foo_dj/jog_bend.h"
#define NOMINMAX
#include <windows.h>
#include <atomic>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/bpm_provider.h"
#include "../sdk/foobar2000/foo_dj/deck_queue.h"
inline std::atomic<bool> g_english{false};
struct Config{int v=0;int get(){return v;}};inline Config cfg_bpmProvider;
namespace pfc{inline GUID guid_null{};}
struct popup_message{static inline int count=0;static void g_show(const char*,const char*){++count;}};
struct Handle{std::string path;unsigned subsong;};
struct metadb_handle_list{std::vector<Handle> items;void add_item(Handle h){items.push_back(h);}};
struct metadb{static metadb* get(){static metadb m;return &m;}Handle handle_create(const char* p,unsigned s){return {p,s};}};
struct contextmenu_item{std::vector<GUID> ids;unsigned get_num_items(){return (unsigned)ids.size();}GUID get_item_guid(unsigned n){return ids[n];}};
inline std::vector<contextmenu_item*> services;
template<class T>struct service_enum_t{auto begin(){return services.begin();}auto end(){return services.end();}};
inline int releases=0;
namespace menu_helpers{inline GUID last{};inline bool succeeds=true,throws=false;inline int calls=0;inline Handle track;inline bool run_command_context(GUID id,GUID,const metadb_handle_list& tracks){assert(releases>0);++calls;last=id;track=tracks.items[0];if(throws)throw std::runtime_error("test analyser failure");return succeeds;}}
inline int answer=IDCANCEL,questions=0;
inline int TestMessageBox(HWND,const wchar_t*,const wchar_t*,UINT){++questions;return answer;}
#define MessageBoxW TestMessageBox
struct CDjPanel{ public: dj_jog::Bend m_jogBend[2];
 bool m_pendingLoad[2]={};std::string m_savedPath[2]={"test://track",""};unsigned m_savedSubsong[2]={7,0};int refreshed=0;
 void AnalyseDeck(int,int);void ReleaseForTags(int){++releases;}void RefreshDeckTags(int,bool=false){++refreshed;}
 std::vector<dj_queue::Item> m_queue[2];int m_queueCurrent[2]={-1,-1};bool m_chain[2]={true,true},m_previewOwned=false;int m_previewDeck=-1;
 struct Window{HWND m_hWnd=nullptr;}m_queueWindow[2];int cancelled=0,dropped=0,stopped=0,saved=0,updated=0,loaded=0;
 void CancelAutomation(){++cancelled;}void DropPreparation(int){++dropped;}void StopPreview(){++stopped;}void SaveSession(){++saved;}void UpdateQueue(int,const std::vector<size_t>& ={}){++updated;}void LoadDeck(int,const char*,unsigned){++loaded;}
 void QueueAction(int,int,std::vector<size_t>,size_t);
};
