#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace dj_auto {
inline double remaining(double length,double position,double speed) {
    if(!std::isfinite(length)||length<=0||!std::isfinite(position)||!std::isfinite(speed)||speed<=0)return -1;
    return (std::max)(0.0,(length-position)/speed);
}
struct Fade {
    bool active=false,finished=false;
    int from=0,to=1;
    uint64_t frames=0,done=0;
    double start=0,end=1;
    void begin(int source,double x,double seconds,unsigned rate) {
        from=source;to=1-source;start=x;end=double(to);done=0;
        frames=uint64_t((std::max)(0.0,seconds)*rate);active=frames>0;finished=!active;
    }
    double advance(size_t count,bool playing) {
        if(active&&playing){done=(std::min)(frames,done+count);if(done==frames){active=false;finished=true;}}
        return frames?start+(end-start)*(double(done)/frames):end;
    }
};
inline void gains(double x,bool linear,double* out) {
    x=(std::max)(0.0,(std::min)(1.0,x));
    out[0]=linear?1-x:std::cos(x*1.5707963267948966);
    out[1]=linear?x:std::sin(x*1.5707963267948966);
}
}
