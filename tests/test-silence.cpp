#define NOMINMAX
#include <cassert>
#include <vector>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/silence_trim.h"
struct Abort{bool cancelled=false;void check(){if(cancelled)throw std::runtime_error("abort");}};
struct Chunk{std::vector<double> data;unsigned get_sample_rate()const{return 1000;}unsigned get_channel_count()const{return 2;}size_t get_sample_count()const{return data.size()/2;}const double* get_data()const{return data.data();}};
struct Reader{
    std::vector<double> samples;size_t position=0;bool seekable=true;
    bool can_seek()const{return seekable;}void seek(double seconds,Abort& abort){abort.check();position=(size_t)std::llround(seconds*1000);}
    bool run(Chunk& out,Abort& abort){abort.check();size_t frames=samples.size()/2;if(position>=frames)return false;auto count=(std::min)(size_t(256),frames-position);out.data.assign(samples.begin()+position*2,samples.begin()+(position+count)*2);position+=count;return true;}
};
int main(){using namespace dj_silence;assert(std::abs(amplitude(-60)-.001)<1e-12);Reader input;input.samples.resize(20000);
    for(size_t n=2000;n<7000;++n)if(n<4000||n>=5000)input.samples[n*2+1]=.2; // one audible channel, internal silence retained
    Abort abort;auto b=analyze<Chunk>(input,10,{true,true,-60},abort);assert(std::abs(b.start-1.98)<1e-9&&std::abs(b.end-7.02)<1e-9&&b.headFound&&b.tailFound);
    b=analyze<Chunk>(input,10,{true,false,-60},abort);assert(std::abs(b.start-1.98)<1e-9&&b.end==10);
    b=analyze<Chunk>(input,10,{false,true,-60},abort);assert(b.start==0&&std::abs(b.end-7.02)<1e-9);
    input.samples.assign(20000,0);b=analyze<Chunk>(input,10,{true,true,-60},abort);assert(b.start==0&&b.end==10&&!b.headFound&&!b.tailFound);
    input.samples.assign(20000,.0005);for(size_t n=2000;n<7000;++n)input.samples[n*2]=.2;
    b=analyze<Chunk>(input,10,{true,true,-80},abort);assert(b.start==0&&b.end==10);
    input.seekable=false;b=analyze<Chunk>(input,10,{true,true,-60},abort);assert(b.start==0&&b.end==10);input.seekable=true;
    b=analyze<Chunk>(input,0,{true,true,-60},abort);assert(b.start==0&&b.end==0);
    abort.cancelled=true;bool failed=false;try{analyze<Chunk>(input,10,{true,true,-60},abort);}catch(...){failed=true;}assert(failed);
    assert(cap(7,6.99,1,1000,512)==10||cap(7,6.99,1,1000,512)==11);assert(cap(7,7,1,1000,512)==0);
    assert(cap(7,6,2,1000,512)==500);assert(cap(7,6,.5,1000,512)==512);
    std::cout<<"Silence trim: -60dBFS, all channels, guarded edges, internal pauses, all-silent preservation, thresholds, unsupported input, cancellation and speed-aware end cap passed\n";
}
