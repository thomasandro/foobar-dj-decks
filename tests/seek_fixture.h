#include "../sdk/foobar2000/foo_dj/jog_bend.h"
#include "../sdk/foobar2000/foo_dj/loop_storage.h"
#include <stdexcept>
#include "../sdk/foobar2000/foo_dj/ab_loop.h"
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <cassert>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/auto_mix.h"
struct abort_callback_dummy{};
namespace pfc::stringcvt{struct string_wide_from_utf8{std::wstring text;explicit string_wide_from_utf8(const char* value){while(*value)text+=(unsigned char)*value++;}const wchar_t* get_ptr()const{return text.c_str();}};}
struct SeekInput{bool seekable=true;double position=0;bool can_seek(){return seekable;}void seek(double value,abort_callback_dummy&){position=value;}};
struct SeekStems{double position=0;void seek(double value){position=value;}};
struct Deck{dj_loop::Region loop;bool loaded=true,playing=false,eof=false,stretchFlushed=false;std::shared_ptr<SeekInput> helper=std::make_shared<SeekInput>();std::shared_ptr<SeekStems> stems;
    std::vector<double> src,pending;double srcPos=0,framesPlayed=0,trimStart=0,trimEnd=0,length=100;size_t pendingPos=0;std::shared_ptr<int> stretch;};
inline int cfg_loopSnapA=0,cfg_loopSnapB=0;
inline std::atomic<bool> g_english{false};
inline std::string loopSavedText;inline bool loopSaveOK=true;inline std::mutex* loopMutex=nullptr;
inline std::string GridKey(const std::string& path,unsigned subsong){picojson::array key{picojson::value(path),picojson::value(double(subsong))};return picojson::value(key).serialize();}
inline bool WriteSessionFile(const std::string& text,const char*){if(loopMutex){assert(loopMutex->try_lock());loopMutex->unlock();}loopSavedText=text;return loopSaveOK;}
struct CDjPanel{ public: dj_jog::Bend m_jogBend[2];
    bool m_pendingLoad[2]={},m_autoChanging=false,m_alternate=true,m_chain[2]={},m_autoArmed[2]={true,true};double m_pendingSeek[2]={-1,-1};
    int m_queueCurrent[2]={3,5},cancelled=0;std::mutex m_lock;Deck m_deck[2];std::wstring m_error[2],m_loopNotice[2];unsigned m_outRate=1000;
    std::atomic<double> m_speed[2]={{1},{1}};std::atomic<float> m_crossfade{0};double m_fadeSeconds=5;dj_auto::Fade m_autoFade;
    void CancelAutomation(){++cancelled;m_alternate=false;m_autoFade={};}
    picojson::object m_loopLibrary;std::string m_savedPath[2]={"file:///music/été.opus","file:///music/été.opus"};unsigned m_savedSubsong[2]={0,1};
    bool m_loopSnap[2]={};bool snapAvailable=true,snapUseGrid=false;double snapTarget=0;
    bool SnapLoopPoint(int,double position,double lower,double upper,double& result){if(!snapAvailable)return false;if(snapUseGrid)return dj_loop::snap(.5,.125,1,position,lower,upper,result);result=snapTarget;return result>=lower&&result<=upper;}
    void SaveLoopPoints(int,const dj_loop::Region&);void SetLoopSnap(int,bool);
    void SeekDeck(int,double);void EditLoop(int,int);void Invalidate(){}
};
