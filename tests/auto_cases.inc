int main(){
    assert(dj_auto::remaining(100,90,2)==5);assert(dj_auto::remaining(100,90,.5)==20);assert(dj_auto::remaining(0,0,1)<0);
    for(unsigned rate:{44100u,48000u,96000u})for(int source:{0,1})for(double seconds:{0.,1.,5.,30.}){
        dj_auto::Fade f;f.begin(source,double(source),seconds,rate);auto before=f.done;f.advance(1024,false);assert(f.done==before);
        if(seconds==0){assert(f.finished&&!f.active);continue;}
        while(f.active){double x=f.advance(1024,true);assert(x>=0&&x<=1);}assert(f.done==f.frames&&f.advance(1,true)==1-source);
    }
    double g[2];dj_auto::gains(.5,false,g);assert(std::abs(g[0]*g[0]+g[1]*g[1]-1)<1e-12);dj_auto::gains(.5,true,g);assert(g[0]==.5&&g[1]==.5);
    {CDjPanel p;p.m_chain[0]=true;p.m_queue[0]={{"a1",0},{"a2",0}};p.m_deck[0].playing=true;p.AutomationTick();
     p.m_deck[0].playing=false;p.AutomationTick();assert(p.loads[0]==0); // manual pause never advances
     p.m_deck[0].eof=true;p.AutomationTick();assert(p.loads[0]==1&&p.m_queueCurrent[0]==1&&p.m_deck[0].playing);
     p.AutomationTick();p.m_deck[0].eof=true;p.m_deck[0].playing=false;p.AutomationTick();p.AutomationTick();assert(p.loads[0]==1);}
    {CDjPanel p;p.m_chain[0]=p.m_chain[1]=true;for(int i=0;i<2;++i){p.m_queue[i]={{"first",0},{"second",0}};p.m_autoArmed[i]=true;p.m_deck[i].eof=true;}
     p.AutomationTick();assert(p.loads[0]==1&&p.loads[1]==1&&p.m_deck[0].playing&&p.m_deck[1].playing);}
    {CDjPanel p;p.m_alternate=true;for(int i=0;i<2;++i)p.m_queue[i]={{"first",0},{"second",0}};
     p.m_deck[0].playing=true;p.m_deck[0].framesPlayed=9500;p.AutomationTick();assert(p.m_autoFade.active&&p.m_deck[1].playing);
     p.m_deck[0].playing=p.m_deck[1].playing=false;p.audio(250);assert(p.m_autoFade.done==0);p.AutomationTick();assert(p.loads[0]==0);
     p.m_deck[0].playing=p.m_deck[1].playing=true;p.audio(250);assert(p.m_crossfade==.5f);p.audio(250);p.AutomationTick();
     assert(p.m_autoActive==1&&p.loads[0]==1&&!p.m_deck[0].playing&&p.m_deck[1].playing);
     p.m_deck[1].framesPlayed=9500;p.AutomationTick();assert(p.m_autoFade.from==1);p.audio(500);p.AutomationTick();assert(p.m_autoActive==0&&p.loads[1]==1);
     p.m_deck[0].framesPlayed=9500;p.AutomationTick();p.audio(500);p.AutomationTick();assert(!p.m_altAvailable[0]&&p.m_autoActive==1);
     p.m_deck[1].eof=true;p.m_deck[1].playing=false;p.AutomationTick();assert(!p.m_alternate&&p.loads[0]==1&&p.loads[1]==1);}
    {CDjPanel p;p.m_alternate=true;p.m_autoArmed[0]=true;p.m_deck[0].eof=true;p.m_deck[0].length=0;p.AutomationTick();assert(p.m_crossfade==1&&p.m_deck[1].playing&&p.m_autoFade.finished);}
    {CDjPanel p;p.m_alternate=true;p.m_deck[1].helper=false;p.AutomationTick();assert(!p.m_alternate&&!p.m_deck[1].playing);}
    {CDjPanel p;p.m_alternate=true;p.m_deck[0].playing=true;p.m_deck[0].framesPlayed=9500;p.AutomationTick();p.audio(100);
     p.m_deck[1].playing=false;p.m_deck[1].eof=true;p.AutomationTick();assert(!p.m_alternate&&p.m_crossfade==0);}
    {CDjPanel p;p.m_alternate=true;p.m_altAvailable[1]=false;p.m_autoArmed[0]=true;p.m_deck[0].eof=true;
     p.m_queue[0]={{"first",0},{"second",0}};p.AutomationTick();assert(p.m_deck[0].playing&&p.loads[0]==1);}
    {CDjPanel p;p.m_chain[0]=true;p.m_queue[0]={{"first",0},{"bad",0}};p.m_autoArmed[0]=true;p.m_deck[0].eof=true;p.AutomationTick();assert(!p.m_chain[0]&&!p.m_deck[0].playing);}
    {CDjPanel p;p.m_alternate=true;p.m_restoring=true;p.m_deck[0].playing=true;p.m_deck[0].framesPlayed=9900;p.AutomationTick();assert(!p.m_deck[1].playing);}
    std::cout<<"Actual automatic playback controller: independent queues, A/B/A exhaustion, pause, EOF, errors, released files, speed-aware fades and audio-frame clocks passed\n";
}
