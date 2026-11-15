#pragma once
#include <cmath>
#include <algorithm>
namespace dj_listening {
// Output seconds, not source position: seeks and tempo cannot earn listening time.
struct Clock {
    double seconds=0;long long started=0;bool submitted=false;
    void consume(size_t frames,unsigned rate,double gain,bool enabled,long long timestamp){
        if(!enabled||!rate||!std::isfinite(gain)||gain<=.001)return;
        if(!started)started=timestamp;
        seconds+=double(frames)/rate;
    }
    bool eligible(double duration)const{return !submitted&&started>0&&std::isfinite(duration)&&duration>30&&seconds>=(std::min)(duration*.5,240.);}
    void discardUnsubmitted(){if(!submitted){seconds=0;started=0;}}
};
inline double safeScale(double scale){return std::isfinite(scale)&&scale>0&&scale<=1000?scale:1.;}
inline double gain(int mode,double track,double album){return mode==1?safeScale(track):mode==2?safeScale(album):1.;}
}
