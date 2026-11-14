#include "../../sdk/foobar2000/foo_dj/auto_mix.h"
#include "../../sdk/foobar2000/foo_dj/deck_queue.h"
#include <atomic>
#include <mutex>
#include <cassert>
#include <iostream>
struct AutoDeck {bool loaded=true,helper=true,playing=false,eof=false;double length=100,framesPlayed=0;size_t pending=0;size_t pendingFrames(){return pending;}};
class CDjPanel {
public:
    bool m_restoring=false,m_autoChanging=false,m_alternate=false,m_chain[2]={},m_autoArmed[2]={},m_altAvailable[2]={true,true};
    int m_autoActive=0,m_queueCurrent[2]={0,0},loads[2]={},saves=0,engine=0;
    unsigned m_outRate=100;double m_fadeSeconds=5;
    std::atomic<double> m_speed[2]={{1},{1}};
    std::atomic<float> m_crossfade{0};std::mutex m_lock;dj_auto::Fade m_autoFade;
    AutoDeck m_deck[2];std::wstring m_error[2];std::vector<dj_queue::Item> m_queue[2];
    void SaveSession(){++saves;}void UpdateQueue(int){}void EnsureEngine(){++engine;}void Invalidate(){}
    void LoadDeck(int deck,const char* path,unsigned){
        ++loads[deck];m_autoArmed[deck]=false;m_deck[deck]=AutoDeck{};
        if(std::string(path)=="bad"){m_deck[deck].loaded=false;m_deck[deck].helper=false;m_error[deck]=L"Decode failure";}
    }
    void CancelAutomation();bool NextQueue(int,bool);void AutomationTick();
    void audio(size_t frames){
        if(m_autoFade.active&&m_deck[m_autoFade.to].playing){m_crossfade=(float)m_autoFade.advance(frames,true);if(m_autoFade.finished)m_deck[m_autoFade.from].playing=false;}
    }
};
