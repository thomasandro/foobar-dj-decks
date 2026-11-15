#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace dj_loop {
// Nearest visible grid line inside the allowed source-time interval.
inline bool snap(double period,double offset,unsigned stride,double position,double lower,double upper,double& target){
    if(!std::isfinite(period)||period<=0||!std::isfinite(offset)||!stride||!std::isfinite(position)||!std::isfinite(lower)||!std::isfinite(upper)||lower>upper)return false;
    const double step=period*stride;if(!std::isfinite(step))return false;
    const double first=std::ceil((lower-offset)/step),last=std::floor((upper-offset)/step);
    if(first>last)return false;
    const double index=(std::max)(first,(std::min)(last,std::round((position-offset)/step)));
    target=offset+index*step;
    return std::isfinite(target)&&target>=lower-1e-9&&target<=upper+1e-9;
}
struct Region {
    double a=-1,b=-1; bool enabled=false;
    bool valid()const{return std::isfinite(a)&&std::isfinite(b)&&a>=0&&b-a>=0.1;}
    bool contains(double t)const{return valid()&&t>=a&&t<b;}
    void setA(double t){a=t;if(!valid()){b=-1;enabled=false;}}
    bool setB(double t){if(!std::isfinite(t)||a<0||t-a<0.1)return false;b=t;return true;}
    // End is exclusive. Round up to the next source-time output frame.
    size_t cap(double position,double speed,unsigned rate,size_t frames)const{
        if(!enabled||!valid()||!rate||!std::isfinite(speed)||speed<=0)return frames;
        double left=(b-position)*rate/speed;
        if(left<=0)return 0;
        return (std::min)(frames,(size_t)std::ceil((std::min)(left,(double)frames)));
    }
};
// Called by the audio worker under the deck mutex, never by a UI timer.
// Clearing decoded/stretch lookahead prevents audio past B leaking into A.
template<class Deck,class Abort> void restart(Deck& d,double seconds,unsigned rate,Abort& abort){
    if(d.stems)d.stems->seek(seconds);else d.helper->seek(seconds,abort);
    d.src.clear();d.srcPos=0;d.pending.clear();d.pendingPos=0;
    d.eof=false;d.stretch.reset();d.stretchFlushed=false;d.framesPlayed=seconds*rate;
}
template<class Deck,class Abort> void wrap(Deck& d,unsigned rate,Abort& abort){
    if(d.loop.enabled&&d.loop.valid()&&!d.loop.contains(d.framesPlayed/rate))restart(d,d.loop.a,rate,abort);
}
}
