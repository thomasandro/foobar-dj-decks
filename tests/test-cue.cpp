#define NOMINMAX
#include <windows.h>
#include <atomic>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <vector>
using audio_sample=double;
struct audio_chunk{static constexpr unsigned channel_config_stereo=3;};
struct audio_chunk_impl{std::vector<double> samples;unsigned rate=0;void set_data(const double* p,size_t n,unsigned ch,unsigned hz,unsigned){assert(ch==2);samples.assign(p,p+2*n);rate=hz;}};
static GUID master={1},phones={2},backend={3},missing={4},broken={5},stalled={6};
struct State{std::mutex mutex;std::vector<double> samples;unsigned rate=0;std::atomic<int> opened{0},closed{0},processed{0};std::atomic<bool> ready{true};} state;
struct output {
    struct ptr:std::shared_ptr<output>{using std::shared_ptr<output>::operator=;bool is_valid()const{return bool(*this);}void release(){reset();}};
    GUID device;
    explicit output(GUID id):device(id){++state.opened;}
    ~output(){++state.closed;}
    unsigned get_forced_sample_rate_(){return 44100;}
    double get_latency(){return .1;}
    void update(bool& ready){if(device==broken)throw std::runtime_error("Disconnected");ready=device!=stalled||state.ready.load();}
    void flush(){}
    void process_samples(const audio_chunk_impl& p){std::lock_guard<std::mutex> lock(state.mutex);state.samples=p.samples;state.rate=p.rate;++state.processed;}
};
struct output_entry {
    static output_entry* g_find(GUID){static output_entry entry;return &entry;}
    unsigned default_bit_depth(GUID){return 24;}
    void instantiate(output::ptr& out,GUID device,double,bool,unsigned){if(device==missing)throw std::runtime_error("Missing headphones");out=std::make_shared<output>(device);}
};
struct output_manager {
    struct Config{GUID m_output=backend,m_device=master;};
    static output_manager* get(){static output_manager manager;return &manager;}Config getCoreConfig(){return {};}
};
#include "../sdk/foobar2000/foo_dj/cue_output.h"
template<class F>bool until(F done){auto start=GetTickCount64();while(GetTickCount64()-start<2000){if(done())return true;Sleep(2);}return done();}
int main(){
    double masterMix[2]={},cueMix[2]={};
    dj_cue::mixFrame(1.,-.5,0.,.5,masterMix,cueMix);
    assert(masterMix[0]==0&&masterMix[1]==0&&cueMix[0]==.5&&cueMix[1]==-.25);
    dj_cue::mixFrame(.2,.4,.7,0.,masterMix,cueMix);
    assert(std::abs(masterMix[0]-.14)<1e-12&&std::abs(masterMix[1]-.28)<1e-12&&cueMix[0]==.5);
    double gains[2];dj_cue::gains(true,false,.5,gains);assert(gains[0]==.25&&gains[1]==0);
    dj_cue::gains(true,true,1,gains);assert(gains[0]==.5&&gains[1]==.5);
    dj_cue::gains(false,false,1,gains);assert(gains[0]==0&&gains[1]==0);
    dj_cue::Queue<double> queue;for(int n=0;n<1000;++n)queue.push({std::vector<double>(960,1.),48000});
    assert(queue.frames<=9600&&queue.discontinuity);queue.clear();assert(queue.frames==0);
    queue.push({std::vector<double>(960,1.),48000});queue.push({std::vector<double>(882,1.),44100});assert(queue.frames==441&&queue.discontinuity);
    // Streaming conversion is invariant to input packet boundaries and stereo separation.
    std::vector<double> input;for(int n=0;n<48000;++n){input.push_back(std::sin(n*.1));input.push_back(-input.back());}
    dj_cue::Resampler<double> whole,stream;auto expected=whole.process(input,48000,44100);std::vector<double> actual;
    for(size_t start=0;start<input.size();start+=1024){auto end=std::min(input.size(),start+1024);auto block=stream.process({input.begin()+start,input.begin()+end},48000,44100);actual.insert(actual.end(),block.begin(),block.end());}
    assert(expected.size()==actual.size()&&expected.size()/2>=44099&&expected.size()/2<=44100);
    for(size_t n=0;n<actual.size();++n)assert(std::abs(expected[n]-actual[n])<1e-7);
    for(size_t n=0;n<actual.size();n+=2)assert(std::abs(actual[n]+actual[n+1])<1e-12);
    dj_cue::Sink sink;std::vector<double> block(960,.25);
    sink.push(block,48000);Sleep(10);assert(state.opened==0); // disabled by default
    sink.configure(true,backend,phones);sink.push(block,48000);assert(until([&]{sink.push(block,48000);return state.processed.load()>0;}));
    {std::lock_guard<std::mutex> lock(state.mutex);assert(state.rate==44100&&!state.samples.empty());for(double sample:state.samples)assert(sample==.25);}
    auto openedBefore=state.opened.load(),playedBefore=state.processed.load();auto oldToken=sink.token();sink.configure(true,backend,phones);
    assert(!sink.push(block,48000,oldToken));assert(until([&]{sink.push(block,48000,sink.token());return state.processed>playedBefore;}));assert(state.opened==openedBefore);
    sink.configure(false,backend,phones);assert(until([]{return state.closed==state.opened;}));
    auto processed=state.processed.load();sink.configure(true,backend,missing);sink.push(block,48000);assert(until([&]{sink.push(block,48000);return !sink.lastError().empty();}));assert(state.processed==processed);
    sink.configure(true,backend,master);sink.push(block,48000);assert(until([&]{sink.push(block,48000);return !sink.lastError().empty();}));assert(state.processed==processed);
    sink.configure(true,backend,broken);sink.push(block,48000);assert(until([&]{sink.push(block,48000);return sink.lastError()=="Disconnected";}));
    state.ready=false;sink.configure(true,backend,stalled);sink.push(block,48000);assert(until([&]{sink.push(block,48000);return state.opened>state.closed;}));
    auto start=GetTickCount64();for(int n=0;n<1000;++n)sink.push(block,48000);assert(GetTickCount64()-start<1000); // stalled output never waits in producer
    sink.configure(false,backend,stalled);assert(until([]{return state.opened==state.closed;}));
    sink.configure(true,backend,phones);sink.push(block,48000);assert(until([&]{sink.push(block,48000);return state.processed>processed;}));assert(sink.lastError().empty());
    sink.stop();assert(state.opened==state.closed);
    std::cout<<"Actual cue sink with mock outputs: routing, independent rate, bounded queue, stall isolation, missing/same/broken devices, recovery and clean shutdown passed; no physical audio opened\n";
}
