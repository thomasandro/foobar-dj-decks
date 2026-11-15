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
struct Deck{bool loaded=true,playing=false,eof=false,stretchFlushed=false;std::shared_ptr<SeekInput> helper=std::make_shared<SeekInput>();std::shared_ptr<SeekStems> stems;
    std::vector<double> src,pending;double srcPos=0,framesPlayed=0,trimStart=0,trimEnd=0,length=100;size_t pendingPos=0;std::shared_ptr<int> stretch;};
inline std::atomic<bool> g_english{false};
struct CDjPanel{
    bool m_pendingLoad[2]={},m_autoChanging=false,m_alternate=true,m_chain[2]={},m_autoArmed[2]={true,true};double m_pendingSeek[2]={-1,-1};
    int m_queueCurrent[2]={3,5},cancelled=0;std::mutex m_lock;Deck m_deck[2];std::wstring m_error[2];unsigned m_outRate=1000;
    std::atomic<double> m_speed[2]={{1},{1}};std::atomic<float> m_crossfade{0};double m_fadeSeconds=5;dj_auto::Fade m_autoFade;
    void CancelAutomation(){++cancelled;m_alternate=false;m_autoFade={};}
    void SeekDeck(int,double);
};
