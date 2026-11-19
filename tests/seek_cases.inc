int main(){
    {CDjPanel p;p.m_jogBend[0].pulse(4,2,1000);p.m_jogBend[1].pulse(-4,2,1000);p.m_deck[0].playing=true;p.SeekDeck(0,20);assert(p.m_jogBend[0].value(1000)==0&&p.m_jogBend[1].value(1000)==-.02);assert(p.m_alternate&&p.cancelled==0&&p.m_deck[0].playing&&!p.m_deck[1].playing);
     assert(p.m_deck[0].framesPlayed==20000&&p.m_queueCurrent[0]==3&&p.m_queueCurrent[1]==5&&p.m_crossfade==0);}
    {CDjPanel p;p.m_chain[0]=true;p.m_deck[0].eof=true;p.SeekDeck(0,0);assert(p.m_alternate&&p.m_chain[0]&&!p.m_deck[0].playing&&!p.m_deck[0].eof);}
    {CDjPanel p;p.m_pendingLoad[0]=true;p.SeekDeck(0,25);assert(p.m_pendingSeek[0]==25&&p.m_alternate&&p.cancelled==0);}
    {CDjPanel p;p.m_deck[0].loaded=false;p.SeekDeck(0,25);assert(p.m_alternate&&p.cancelled==0);}
    {CDjPanel p;p.m_deck[0].playing=p.m_deck[1].playing=true;p.m_autoFade.begin(0,0,5,1000);p.m_crossfade=.4f;
     p.SeekDeck(0,50);assert(p.m_alternate&&p.m_autoFade.active&&p.m_autoFade.from==0&&p.m_autoFade.start==p.m_crossfade.load());
     assert(p.m_deck[0].playing&&p.m_deck[1].playing&&p.m_queueCurrent[0]==3&&p.m_queueCurrent[1]==5);
     p.SeekDeck(1,99);assert(p.m_autoFade.frames==1000&&p.m_alternate&&p.m_autoFade.start==p.m_crossfade.load());}
    {CDjPanel p;p.m_deck[0].trimStart=2;p.m_deck[0].trimEnd=80;p.SeekDeck(0,0);assert(p.m_deck[0].framesPlayed==2000&&p.m_alternate);}
    {CDjPanel p;p.m_deck[0].loop={10,20,true};p.SeekDeck(0,15);assert(p.m_deck[0].loop.enabled);p.SeekDeck(0,25);assert(!p.m_deck[0].loop.enabled&&p.m_alternate);}
    std::cout<<"Actual SeekDeck: AUTO mode, queues and play/pause preserved; pending/empty decks, trimmed start and continuous active-fade replanning passed (mock decoders)\n";
}
