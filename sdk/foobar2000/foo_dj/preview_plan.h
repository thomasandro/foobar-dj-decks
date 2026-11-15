#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include <atomic>
namespace dj_preview {
struct Span{double start,seconds;};
inline const wchar_t* name(int mode,bool en){
    static const wchar_t* fr[]={L"8 s du début",L"15 s du début",L"8 s du milieu",L"15 s du milieu",L"8 s du début + 8 s du milieu"};
    static const wchar_t* eng[]={L"First 8 seconds",L"First 15 seconds",L"Middle 8 seconds",L"Middle 15 seconds",L"First 8 + middle 8 seconds"};
    return (en?eng:fr)[mode>=0&&mode<5?mode:0];
}
inline std::vector<Span> plan(double length,int mode){
    mode=mode>=0&&mode<5?mode:0;const bool known=std::isfinite(length)&&length>0;
    if(mode>=2&&!known)throw std::runtime_error("Preview: track duration is unknown / durée inconnue");
    double seconds=(mode==1||mode==3)?15.:8.;std::vector<Span> result;
    if(mode<2||mode==4)result.push_back({0,known?(std::min)(seconds,length):seconds});
    if(mode>=2){double start=(std::max)(0.,(length-seconds)/2.);
        if(mode==4)start=(std::max)(start,result.front().seconds);
        if(start<length)result.push_back({start,(std::min)(seconds,length-start)});
    }
    return result;
}
inline std::vector<Span> planAt(double length,int mode,double position){
    if(position<0)return plan(length,mode);
    if(!std::isfinite(position)||!std::isfinite(length)||length<=0)throw std::runtime_error("Preview position unavailable / Position indisponible");
    position=std::clamp(position,0.,(std::max)(0.,length-.001));
    double seconds=mode==4?16.:(mode==1||mode==3)?15.:8.;
    return {{position,(std::min)(seconds,length-position)}};
}
// Latest request wins; cancellation does not join a decoding thread on every click.
// The job owns all decoding state. Completion is published only after it releases
// that state, including on error. wait=true is reserved for tag-writing handoff.
template<class Request,class Abort> class Runner {
    std::mutex mutex;std::condition_variable changed;std::thread thread;
    bool stopRequested=false,hasRequest=false,busy=false;size_t serial=0;
    Request pending{};std::shared_ptr<Abort> abort;std::atomic<bool> running{false};std::string error;
    std::function<void(const Request&,Abort&)> play;
    void run(){while(true){Request request;std::shared_ptr<Abort> cancel;size_t token;
        {std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[&]{return stopRequested||hasRequest;});if(stopRequested)break;
         request=pending;cancel=abort;token=serial;hasRequest=false;busy=true;}
        std::string failure;try{play(request,*cancel);}catch(const std::exception& e){if(!cancel->is_aborting())failure=e.what();}catch(...){if(!cancel->is_aborting())failure="Preview error / Erreur de préécoute";}
        {std::lock_guard<std::mutex> lock(mutex);busy=false;if(token==serial){running=false;error=failure;}}changed.notify_all();
    }}
public:
    explicit Runner(std::function<void(const Request&,Abort&)> function):play(std::move(function)){}
    ~Runner(){stop();}
    void start(Request request){
        {std::lock_guard<std::mutex> lock(mutex);if(abort)abort->set();++serial;pending=std::move(request);abort=std::make_shared<Abort>();hasRequest=true;running=true;error.clear();}
        if(!thread.joinable())thread=std::thread([this]{run();});changed.notify_all();
    }
    void cancel(bool wait=false){
        std::unique_lock<std::mutex> lock(mutex);if(abort)abort->set();++serial;hasRequest=false;running=false;error.clear();
        if(wait)changed.wait(lock,[&]{return !busy;});
    }
    bool active()const{return running.load();}
    std::string lastError(){std::lock_guard<std::mutex> lock(mutex);return error;}
    void stop(){cancel(true);{std::lock_guard<std::mutex> lock(mutex);stopRequested=true;}changed.notify_all();if(thread.joinable())thread.join();}
};
}
