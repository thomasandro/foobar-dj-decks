int main(){
    {CDjPanel p;p.m_deck[0].framesPlayed=10000;p.EditLoop(0,0);p.EditLoop(0,1);assert(p.m_deck[0].loop.b<0&&!p.m_loopNotice[0].empty()&&p.m_error[0].empty());
     p.m_deck[0].framesPlayed=20000;p.EditLoop(0,1);assert(p.m_deck[0].loop.valid());p.EditLoop(0,2);
     assert(p.m_deck[0].loop.enabled&&!p.m_deck[0].playing&&p.m_deck[0].framesPlayed==10000&&p.m_alternate);
     p.EditLoop(0,0);assert(p.m_deck[0].loop.enabled);p.EditLoop(0,2);assert(!p.m_deck[0].loop.enabled);
     p.EditLoop(0,3);assert(p.m_deck[0].loop.a<0&&p.m_deck[1].loop.a<0);}
    {CDjPanel p;p.m_deck[0].helper->seekable=false;p.EditLoop(0,0);assert(p.m_deck[0].loop.a<0);}
    {CDjPanel p;p.m_deck[0].loop={1,2,false};p.m_autoFade.begin(0,0,5,1000);p.EditLoop(0,2);assert(!p.m_deck[0].loop.enabled);}
    {CDjPanel p;p.m_deck[0].playing=true;p.m_deck[0].loop={1,2,false};p.m_deck[0].src={1,2};p.m_deck[0].pending={3,4};p.EditLoop(0,2);
     assert(p.m_deck[0].playing&&p.m_deck[0].src.empty()&&p.m_deck[0].pending.empty()&&p.m_deck[0].framesPlayed==1000);}
    {CDjPanel p;p.m_deck[0].loop={1,2,false};p.m_autoFade.finished=true;p.EditLoop(0,2);assert(!p.m_deck[0].loop.enabled&&p.m_alternate);}
    {CDjPanel p;p.m_error[0]=L"decode failure";p.EditLoop(0,3);assert(p.m_error[0]==L"decode failure");}
    {CDjPanel p;loopMutex=&p.m_lock;p.m_deck[0].framesPlayed=12500;p.EditLoop(0,0);
     auto first=dj_loop::restore(p.m_loopLibrary,GridKey(p.m_savedPath[0],0),0,100);assert(first.a==12.5&&first.b<0&&!first.enabled);
     p.m_deck[0].framesPlayed=25000;p.EditLoop(0,1);picojson::value disk;assert(picojson::parse(disk,loopSavedText).empty());
     auto restored=dj_loop::restore(disk.get<picojson::object>(),GridKey(p.m_savedPath[0],0),0,100);assert(restored.a==12.5&&restored.b==25&&!restored.enabled);
     p.m_deck[1].framesPlayed=5000;p.EditLoop(1,0);assert(p.m_loopLibrary.size()==2);
     p.EditLoop(0,3);assert(p.m_loopLibrary.size()==1&&p.m_loopLibrary.count(GridKey(p.m_savedPath[1],1)));
     loopSaveOK=false;p.m_deck[1].framesPlayed=15000;p.EditLoop(1,1);assert(!p.m_loopNotice[1].empty()&&p.m_error[1].empty());loopSaveOK=true;loopMutex=nullptr;}
    {CDjPanel p;p.m_loopSnap[0]=true;p.snapTarget=12.75;p.m_deck[0].framesPlayed=12600;p.EditLoop(0,0);assert(p.m_deck[0].loop.a==12.75);
     p.snapTarget=14.25;p.m_deck[0].framesPlayed=14100;p.EditLoop(0,1);assert(p.m_deck[0].loop.b==14.25);
     assert(dj_loop::restore(p.m_loopLibrary,GridKey(p.m_savedPath[0],0),0,100).b==14.25);
     p.snapAvailable=false;p.EditLoop(0,1);assert(p.m_deck[0].loop.b==14.25&&!p.m_loopNotice[0].empty());}
    {CDjPanel p;p.snapUseGrid=true;p.m_deck[0].loop={10.2,12.3,false};p.m_deck[0].framesPlayed=11000;p.SetLoopSnap(0,true);
     assert(p.m_loopSnap[0]&&cfg_loopSnapA==1&&p.m_deck[0].loop.a==10.125&&p.m_deck[0].loop.b==12.125&&!p.m_deck[0].playing);
     auto saved=dj_loop::restore(p.m_loopLibrary,GridKey(p.m_savedPath[0],0),0,100);assert(saved.a==10.125&&saved.b==12.125);
     p.SetLoopSnap(0,false);assert(!p.m_loopSnap[0]&&p.m_deck[0].loop.a==10.125);}
    {CDjPanel p;p.snapUseGrid=true;p.m_deck[0].loop={10.2,12.3,true};p.m_deck[0].playing=true;p.m_deck[0].framesPlayed=12200;p.SetLoopSnap(0,true);
     assert(p.m_deck[0].loop.enabled&&p.m_deck[0].playing&&p.m_deck[0].framesPlayed==10125&&p.m_alternate);}
    {CDjPanel p;p.snapAvailable=false;p.m_deck[0].loop={10.2,12.3,false};p.SetLoopSnap(0,true);assert(!p.m_loopSnap[0]&&p.m_deck[0].loop.a==10.2&&p.m_deck[0].loop.b==12.3&&!p.m_loopNotice[0].empty());}
    {CDjPanel p;p.snapUseGrid=true;p.m_deck[0].loop={10.2,-1,false};p.SetLoopSnap(0,true);assert(p.m_deck[0].loop.a==10.125&&p.m_deck[0].loop.b<0);}
    {CDjPanel p;p.snapUseGrid=true;p.m_deck[0].length=12.2;p.m_deck[0].loop={12.05,12.18,false};p.SetLoopSnap(0,true);assert(p.m_loopSnap[0]&&p.m_deck[0].loop.a==11.625&&p.m_deck[0].loop.b==12.125);}
    std::cout<<"Actual A/B loop controls: independent points, invalid bounds, pause/play retention, buffer reset, unavailable seeking and active AUTO fade guard passed\n";
}
