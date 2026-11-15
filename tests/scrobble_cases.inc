int main(){
    {CDjPanel p;p.qualify(0);g_scrobble=false;p.tick();assert(dj_scrobble::backend->calls==0&&p.m_deck[0].listening.seconds==0);g_scrobble=true;}
    {CDjPanel p;p.qualify(0);dj_scrobble::enabled=false;p.tick();assert(dj_scrobble::backend->calls==0&&!p.m_scrobbleAllowed[0]);dj_scrobble::enabled=true;}
    {CDjPanel p;p.qualify(0);dj_scrobble::available=false;p.tick();assert(dj_scrobble::backend->calls==0);dj_scrobble::available=true;}
    {CDjPanel p;p.qualify(0);p.tick();p.tick();assert(dj_scrobble::backend->calls==1&&p.m_deck[0].listening.submitted);}
    {CDjPanel p;p.qualify(0);playback_control::get()->normal=metadb::get()->handle_create("a",0);p.tick();assert(dj_scrobble::backend->calls==1&&p.m_deck[0].listening.seconds==0&&!p.m_scrobbleAllowed[0]);playback_control::get()->normal={};}
    {CDjPanel p;p.qualify(0);auto h=metadb::get()->handle_create("a",0);h->info.artist.clear();p.tick();assert(dj_scrobble::backend->calls==1);h->info.artist="artist";}
    {CDjPanel p;p.qualify(0);p.qualify(1);p.tick();assert(dj_scrobble::backend->calls==3);}
    {CDjPanel p;p.qualify(0);dj_scrobble::backend->fail=true;p.tick();p.tick();assert(dj_scrobble::backend->calls==4&&p.m_deck[0].listening.submitted);dj_scrobble::backend->fail=false;}
    std::cout<<"Actual ScrobbleTick with mock backend: opt-in, host toggle, missing service/tags, main-player suppression, independent decks and no repeated submission after success/failure passed; no real scrobbles sent\n";
}
