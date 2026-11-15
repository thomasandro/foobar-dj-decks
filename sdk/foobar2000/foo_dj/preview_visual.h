#pragma once
#include "preview_plan.h"
#include <array>
namespace dj_preview {
struct VisualState {
    static constexpr size_t bins=1024;
    std::array<float,bins> peaks{};std::array<bool,bins> known{};
    std::vector<Span> spans;double duration=0,trackDuration=0,submitted=0,position=0,sourcePosition=0;
    bool overviewDone=false;
};
class Visual {
    std::mutex mutex;size_t token=0;VisualState state;
public:
    void reset(size_t generation){std::lock_guard<std::mutex> lock(mutex);token=generation;state={};}
    void restart(size_t generation,double sourcePosition){
        std::lock_guard<std::mutex> lock(mutex);token=generation;state.spans.clear();
        state.duration=state.submitted=state.position=0;state.sourcePosition=sourcePosition;
    }
    void begin(size_t generation,const std::vector<Span>& spans,double length=0){
        std::lock_guard<std::mutex> lock(mutex);if(generation!=token)return;state.spans=spans;state.duration=0;state.submitted=0;
        for(auto span:spans)state.duration+=span.seconds;
        if(std::isfinite(length)&&length>0)state.trackDuration=length;
        if(state.trackDuration<=0)state.trackDuration=state.duration;
    }
    template<class Sample>void add(size_t generation,double start,unsigned rate,const Sample* data,size_t frames,unsigned channels){
        std::lock_guard<std::mutex> lock(mutex);if(generation!=token||state.trackDuration<=0||!rate||!channels||!data)return;
        for(size_t n=0;n<frames;++n){double time=start+(double)n/rate;if(time<0||time>=state.trackDuration)continue;
            auto bin=(std::min)(VisualState::bins-1,(size_t)(time/state.trackDuration*VisualState::bins));
            double peak=0;for(unsigned c=0;c<(std::min)(channels,2u);++c){double v=std::abs((double)data[n*channels+c]);if(std::isfinite(v))peak=(std::max)(peak,v);}
            state.peaks[bin]=(std::max)(state.peaks[bin],(float)(std::min)(1.,peak));state.known[bin]=true;
        }
    }
    void overview(size_t generation,double length,const std::array<float,32>& peaks,const std::array<bool,32>& known,bool done){
        std::lock_guard<std::mutex> lock(mutex);if(generation!=token||!std::isfinite(length)||length<=0)return;
        state.trackDuration=length;state.overviewDone=done;
        for(size_t n=0;n<state.bins;++n){auto cell=n*32/state.bins;if(known[cell]){state.peaks[n]=(std::max)(state.peaks[n],peaks[cell]);state.known[n]=true;}}
    }
    void submitted(size_t generation,double seconds){std::lock_guard<std::mutex> lock(mutex);if(generation==token)state.submitted=seconds;}
    VisualState snapshot(double latency){
        std::lock_guard<std::mutex> lock(mutex);auto result=state;
        if(!std::isfinite(latency)||latency<0)latency=0;
        result.position=std::clamp(result.submitted-latency,0.,result.duration);
        double elapsed=0;for(auto span:result.spans){result.sourcePosition=span.start+std::clamp(result.position-elapsed,0.,span.seconds);elapsed+=span.seconds;if(result.position<elapsed)break;}
        return result;
    }
};
}
