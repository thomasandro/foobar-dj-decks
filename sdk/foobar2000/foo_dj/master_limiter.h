#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace dj_master {
// Zero-lookahead sample-peak limiter: linked stereo, immediate attack, 80 ms
// exponential release. No allocation, host calls or locks in the sample loop.
struct Meter {double input=0,output=0,reduction=0;bool invalid=false;};
class Limiter {
    double gain=1;
public:
    void reset(){gain=1;}
    template<class Sample> Meter process(Sample* stereo,size_t frames,unsigned rate,bool enabled,double ceilingDb=-1){
        Meter meter;double lowest=1;
        ceilingDb=std::isfinite(ceilingDb)?std::clamp(ceilingDb,-12.,-.1):-1.;
        const double ceiling=std::pow(10.,ceilingDb/20.),release=std::exp(-1./(.080*(rate?rate:48000)));
        if(!enabled)gain=1;
        for(size_t n=0;n<frames;++n){
            double l=stereo[n*2],r=stereo[n*2+1];
            if(!std::isfinite(l)){l=0;meter.invalid=true;}if(!std::isfinite(r)){r=0;meter.invalid=true;}
            const double peak=(std::max)(std::abs(l),std::abs(r));meter.input=(std::max)(meter.input,peak);
            if(enabled){double target=peak>ceiling?ceiling/peak:1.;gain=(std::min)(target,1.-(1.-gain)*release);}
            lowest=(std::min)(lowest,gain);
            // Clamp rounding at the chosen ceiling; stereo uses one common gain.
            l*=gain;r*=gain;if(enabled){l=std::clamp(l,-ceiling,ceiling);r=std::clamp(r,-ceiling,ceiling);}
            stereo[n*2]=(Sample)l;stereo[n*2+1]=(Sample)r;
            meter.output=(std::max)(meter.output,(std::max)(std::abs(double(stereo[n*2])),std::abs(double(stereo[n*2+1]))));
        }
        meter.reduction=-20.*std::log10((std::max)(lowest,1e-30));return meter;
    }
};
}
