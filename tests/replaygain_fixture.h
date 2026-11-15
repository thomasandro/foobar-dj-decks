#include <cmath>
#include <cassert>
#include <iostream>
using audio_sample=double;
namespace audio_math {inline double gain_to_scale(double dB){return std::pow(10.,dB/20.);}}
struct replaygain_info {
    float m_track_gain=-1000,m_album_gain=-1000,m_track_peak=-1,m_album_peak=-1;
    bool is_track_gain_present()const{return m_track_gain!=-1000;}bool is_album_gain_present()const{return m_album_gain!=-1000;}
    bool is_track_peak_present()const{return m_track_peak!=-1;}bool is_album_peak_present()const{return m_album_peak!=-1;}
};
struct t_replaygain_config {
    enum{source_mode_track=1,source_mode_album=2,processing_mode_gain=1,processing_mode_gain_and_peak=2,processing_mode_peak=3};
    int m_source_mode=1,m_processing_mode=2;double m_preamp_with_rg=0,m_preamp_without_rg=0;
    audio_sample query_scale(const replaygain_info&)const;
};
