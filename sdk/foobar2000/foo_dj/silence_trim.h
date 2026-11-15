#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <stdexcept>
namespace dj_silence {
struct Options{bool head=false,tail=false;int db=-60;bool enabled()const{return head||tail;}
    std::string key()const{return std::to_string(head)+std::to_string(tail)+":"+std::to_string(db);}};
struct Bounds{double start=0,end=0;bool headFound=false,tailFound=false;};
inline double amplitude(int db){return std::pow(10.,std::clamp(db,-120,-1)/20.);}
template<class Chunk>std::pair<double,double> audible(const Chunk& chunk,double offset,double end,double threshold){
    unsigned rate=chunk.get_sample_rate(),channels=chunk.get_channel_count();
    if(!rate||!channels||channels>256)throw std::runtime_error("Silence: invalid audio format");
    auto samples=chunk.get_data();double first=-1,last=-1;
    for(size_t n=0;n<chunk.get_sample_count();++n){double time=offset+(double)n/rate;if(time>=end)break;
        bool sound=false;for(unsigned c=0;c<channels;++c){double sample=samples[n*channels+c];if(!std::isfinite(sample)||std::abs(sample)>threshold){sound=true;break;}}
        if(sound){if(first<0)first=time;last=time+1./rate;}
    }
    return {first,last};
}
// Exhaustive sample detection only within the first/last 60s, never sparse
// overview peaks. Preserve 20ms guards. If no sound is found, retain that edge.
template<class Chunk,class Reader,class Abort>Bounds analyze(Reader& input,double length,Options options,Abort& abort){
    Bounds bounds;bounds.end=length;
    if(!options.enabled()||!std::isfinite(length)||length<=0||!input.can_seek())return bounds;
    const double threshold=amplitude(options.db),guard=.02;
    if(options.head){input.seek(0,abort);double position=0,limit=(std::min)(length,60.);
        while(position<limit){abort.check();Chunk chunk;if(!input.run(chunk,abort))break;if(!chunk.get_sample_count())break;
            auto found=audible(chunk,position,limit,threshold);
            if(found.first>=0){bounds.start=(std::max)(0.,found.first-guard);bounds.headFound=true;break;}
            position+=(double)chunk.get_sample_count()/chunk.get_sample_rate();}
    }
    if(options.tail){double end=length,limit=(std::max)(0.,length-60.);
        while(end>limit){abort.check();double start=(std::max)(limit,end-5.),position=start,last=-1;input.seek(start,abort);
            while(position<end){Chunk chunk;if(!input.run(chunk,abort))break;abort.check();if(!chunk.get_sample_count())break;
                auto found=audible(chunk,position,end,threshold);if(found.second>=0)last=found.second;
                position+=(double)chunk.get_sample_count()/chunk.get_sample_rate();}
            if(last>=0){bounds.end=(std::min)(length,last+guard);bounds.tailFound=true;break;}end=start;}
    }
    if(bounds.end<=bounds.start){bounds.start=0;bounds.end=length;bounds.headFound=bounds.tailFound=false;}
    return bounds;
}
inline size_t cap(double end,double position,double speed,unsigned rate,size_t available){
    if(!std::isfinite(end)||end<=0||!std::isfinite(position)||!std::isfinite(speed)||speed<=0||!rate)return available;
    if(position>=end)return 0;
    double frames=std::ceil((end-position)*rate/speed);return frames>=available?available:(size_t)frames;
}
}
