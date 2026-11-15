#pragma once
#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>

namespace dj_cue {
template<class Sample> inline void mixFrame(Sample left,Sample right,Sample masterGain,Sample cueGain,Sample* master,Sample* cue){
    master[0]+=left*masterGain;master[1]+=right*masterGain;
    cue[0]+=left*cueGain;cue[1]+=right*cueGain;
}
inline void gains(bool a,bool b,double volume,double* out){
    volume=std::isfinite(volume)?std::clamp(volume,0.,1.):0.;
    const double g=volume*volume/((a&&b)?2.:1.);
    out[0]=a?g:0.;out[1]=b?g:0.;
}
// Separate device clocks must never back-pressure the master. Keep at most 200ms
// pending and discard stale audio on overflow rather than accumulate seconds.
template<class Sample> struct Packet {std::vector<Sample> audio;unsigned rate=0;};
template<class Sample> struct Queue {
    std::deque<Packet<Sample>> packets;size_t frames=0;bool discontinuity=false;
    void clear(){packets.clear();frames=0;discontinuity=false;}
    void push(Packet<Sample> p){
        const size_t n=p.audio.size()/2,limit=std::max<size_t>(1,p.rate/5);
        if(!p.rate||!n||n>limit)return;
        if(!packets.empty()&&packets.back().rate!=p.rate){clear();discontinuity=true;}
        while(frames+n>limit&&!packets.empty()){frames-=packets.front().audio.size()/2;packets.pop_front();discontinuity=true;}
        frames+=n;packets.push_back(std::move(p));
    }
    Packet<Sample> pop(){auto p=std::move(packets.front());packets.pop_front();frames-=p.audio.size()/2;return p;}
};
template<class Sample> struct Resampler {
    std::vector<Sample> pending;double position=0;unsigned inputRate=0,outputRate=0;
    void reset(){pending.clear();position=0;inputRate=outputRate=0;}
    std::vector<Sample> process(const std::vector<Sample>& input,unsigned in,unsigned out){
        if(in<1000||out<1000||in>768000||out>768000||input.size()%2)return {};
        if(inputRate!=in||outputRate!=out){reset();inputRate=in;outputRate=out;}
        if(in==out)return input;
        pending.insert(pending.end(),input.begin(),input.end());std::vector<Sample> result;
        const size_t frames=pending.size()/2;const double step=(double)in/out;
        while(position+1<frames){auto index=(size_t)position;double t=position-index;
            for(int c=0;c<2;++c)result.push_back((Sample)(pending[index*2+c]*(1-t)+pending[(index+1)*2+c]*t));
            position+=step;
        }
        if(frames){auto used=(std::min)((size_t)position,frames-1);pending.erase(pending.begin(),pending.begin()+used*2);position-=used;}
        return result;
    }
};
}
