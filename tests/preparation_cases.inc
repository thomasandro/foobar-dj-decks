template<class F>bool until(F done){auto start=GetTickCount64();while(GetTickCount64()-start<3000){if(done())return true;Sleep(1);}return done();}
int main(){
    CDjPanel p;p.LoadDeck(0,"one",0);assert(p.m_pendingLoad[0]&&!p.m_deck[0].playing);
    assert(until([&]{p.PreparationTick();return !p.m_pendingLoad[0];}));
    assert(p.m_deck[0].loaded&&!p.m_deck[0].playing&&std::abs(p.m_deck[0].trimStart-1.98)<1e-6&&std::abs(p.m_deck[0].trimEnd-7.02)<1e-6);
    assert(p.m_deck[0].src.size()>=500&&p.m_deck[0].framesPlayed>90000);
    p.LoadDeck(0,"two",0);p.m_pendingPlay[0]=true;p.m_pendingSeek[0]=3;
    assert(until([&]{p.PreparationTick();return !p.m_pendingLoad[0];}));assert(p.m_deck[0].playing&&p.sought[0]==3&&p.engine==1);
    p.m_chain[0]=true;p.m_queue[0]={{"two",0},{"three",0}};p.PreparationTick();
    assert(until([&]{return p.NextPrepared(0);}));auto ready=p.TakePreparation(0,{"three",0});assert(ready&&ready->deck.helper&&!ready->deck.src.empty());
    p.ClearPreparations();p.m_prepareSuspended[0]=true;p.PreparationTick();assert(!p.NextPrepared(0));
    p.m_prepareSuspended[0]=false;p.LoadDeck(0,"bad",0);assert(until([&]{p.PreparationTick();return !p.m_pendingLoad[0];}));assert(!p.m_deck[0].loaded&&!p.m_error[0].empty());
    p.LoadDeck(0,"nonstd",0);assert(until([&]{p.PreparationTick();return !p.m_pendingLoad[0];}));assert(!p.m_deck[0].loaded&&!p.m_error[0].empty());
    p.ClearPreparations(true);options={false,false,-60};auto before=p.loads[0];p.LoadDeck(0,"immediate",0);assert(p.loads[0]==before+1&&!p.m_pendingLoad[0]);
    std::cout<<"Actual preparation scheduler/load wrapper: asynchronous guarded trim, paused adoption, play/seek intent, warm next track, suspended release, decode error and disabled legacy load passed (mock decoder, no files/audio)\n";
}
