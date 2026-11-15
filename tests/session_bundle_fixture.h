#pragma once
#include "../sdk/foobar2000/foo_dj/session_bundle.h"
#include "../sdk/foobar2000/foo_dj/loop_storage.h"
#include "../sdk/foobar2000/foo_dj/beat_sync.h"
#include <atomic>
#include <mutex>
#include <map>
#include <cassert>
#include <iostream>
inline int cfg_loopSnapA=0,cfg_loopSnapB=0;
inline std::string GridKey(const std::string& p,unsigned s){picojson::array key{picojson::value(p),picojson::value(double(s))};return picojson::value(key).serialize();}
struct Deck{bool loaded=false,playing=false,helper=false;double framesPlayed=0,length=1000,bpmValue=120;dj_loop::Region loop;};
struct CDjPanel{
 std::mutex m_lock;Deck m_deck[2];std::vector<dj_queue::Item> m_queue[2];int m_queueCurrent[2]={-1,-1};std::string m_savedPath[2];unsigned m_savedSubsong[2]={};unsigned m_outRate=1000;
 bool m_pendingLoad[2]={},m_pendingPlay[2]={},m_prepareSuspended[2]={},m_applyImport[2]={},m_restoring=false,m_chain[2]={},m_alternate=false,m_manualGrid[2]={},m_stemReady[2]={},m_stemActive[2]={},m_loopSnap[2]={};
 double m_pendingSeek[2]={-1,-1},m_manualPhase[2]={},m_fadeSeconds=5;unsigned long long m_gridStamp[2]={};std::wstring m_name[2],m_artist[2],m_error[2],m_loopNotice[2];
 std::atomic<double> m_speed[2]={{1},{1}};std::atomic<int> m_speedRange[2]={{16},{16}},m_transpose[2]={{0},{0}};std::atomic<bool> m_fixedPitch[2]={{true},{true}},m_cue[2]={{false},{false}},m_linearFade{false};std::atomic<float>m_volume[2]={{1},{1}},m_crossfade{.5f};
 picojson::object m_loopLibrary,m_gridLibrary;std::set<std::string>m_queueInfoAttempted;bool m_forceQueueInfo=false;struct Fade{bool active=false;}m_autoFade;dj_session::Deck m_importDeck[2];
 int cancelled=0,previewStopped=0,cleared=0,stemsCancelled=0,wavesStopped=0,configured=0,loads=0,seeks=0,updates=0,saves=0;bool async=false;std::string loadPath[2];
 void CancelAutomation(){++cancelled;m_alternate=false;m_autoFade={};}void StopPreview(){++previewStopped;}void ClearPreparations(){++cleared;}void CancelStems(){++stemsCancelled;}void StopWaveThread(int){++wavesStopped;}void ConfigureCue(){++configured;}
 void LoadDeck(int i,const char* path,unsigned sub){++loads;loadPath[i]=path;assert(m_importDeck[i].item.path==path);assert(!m_chain[i]&&!m_alternate);if(async){m_pendingLoad[i]=true;return;}if(std::string(path)=="missing"){m_error[i]=L"missing";return;}m_savedPath[i]=path;m_savedSubsong[i]=sub;m_deck[i].loaded=m_deck[i].helper=true;m_deck[i].playing=false;m_deck[i].loop=dj_session::bounded(m_importDeck[i].loop,0,1000);m_manualGrid[i]=m_importDeck[i].manual;m_manualPhase[i]=m_importDeck[i].phase;m_applyImport[i]=false;}
 void SeekDeck(int i,double t){++seeks;if(m_pendingLoad[i])m_pendingSeek[i]=t;else if(m_deck[i].loaded)m_deck[i].framesPlayed=t*m_outRate;}
 void UpdateQueue(int){++updates;}void SaveSession(){assert(!m_restoring);++saves;}void Invalidate(){}
 dj_session::Bundle CaptureBundle(bool);void ApplyBundle(const dj_session::Bundle&,bool);
};
