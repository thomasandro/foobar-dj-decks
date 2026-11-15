#include "../sdk/foobar2000/foo_dj/session_bundle.h"
class CDjPanel{
    bool m_applyImport[2]={};dj_session::Deck m_importDeck[2];

public:
    bool m_autoChanging=false,m_alternate=false,m_chain[2]={};int m_queueCurrent[2]={0,0};std::vector<dj_queue::Item> m_queue[2];
    std::mutex m_lock;Deck m_deck[2];std::wstring m_error[2];unsigned m_outRate=48000;int loads[2]={},engine=0;double sought[2]={-1,-1};
    void CancelAutomation(){m_alternate=false;}void EnsureEngine(){++engine;}void Invalidate(){}
    void SeekDeck(int i,double seconds){sought[i]=seconds;m_deck[i].framesPlayed=seconds*m_outRate;}
    void CommitLoadDeck(int i,const char*,unsigned,std::shared_ptr<PreparedTrack> ready={}){
        ++loads[i];if(ready&&!ready->error.empty()){m_deck[i]={};m_error[i]=L"failed";return;}
        if(ready)m_deck[i]=std::move(ready->deck);else m_deck[i].helper=std::make_shared<input_helper>();
        m_deck[i].loaded=true;m_deck[i].framesPlayed=m_deck[i].trimStart*m_outRate;m_error[i].clear();
    }
    void LoadDeck(int,const char*,unsigned);void PreparationTick();
