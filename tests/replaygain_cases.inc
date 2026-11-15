int main(){
    t_replaygain_config c;replaygain_info r;assert(c.query_scale(r)==1);
    r.m_track_gain=-6;assert(std::abs(c.query_scale(r)-std::pow(10.,-.3))<1e-7);
    r.m_album_gain=-12;c.m_source_mode=2;assert(std::abs(c.query_scale(r)-std::pow(10.,-.6))<1e-7);
    r.m_album_gain=-1000;assert(std::abs(c.query_scale(r)-std::pow(10.,-.3))<1e-7);
    r.m_album_gain=6;r.m_album_peak=.8f;assert(std::abs(c.query_scale(r)-1/.8)<1e-6);
    r.m_track_gain=-1000;r.m_album_gain=-1000;r.m_album_peak=2;assert(c.query_scale(r)==.5);
    r.m_album_peak=-1;r.m_track_peak=-1;r.m_album_gain=-12;c.m_source_mode=1;assert(std::abs(c.query_scale(r)-std::pow(10.,-.6))<1e-7);
    std::cout<<"Actual SDK ReplayGain scale: neutral/missing tags, track/album gain and fallbacks, peak clipping prevention passed\n";
}
