#pragma once
#include "cue_audio.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <stdexcept>
#include <atomic>

namespace dj_cue {
// Only this worker touches its output instance. No call to a driver is made
// while holding the queue mutex or the deck/audio mutex.
class Sink {
    std::mutex mutex;std::condition_variable changed;std::thread worker;
    bool stopped=false,enabled=false;size_t revision=0;
    GUID backend={},device={};Queue<audio_sample> queue;std::string error;
    std::atomic<size_t> serial{0};
    std::atomic<double> buffered{0};
    void measure(output::ptr& out){
        double seconds=out->get_latency();if(!std::isfinite(seconds)||seconds<0)seconds=0;
        {std::lock_guard<std::mutex> lock(mutex);if(!queue.packets.empty())seconds+=(double)queue.frames/queue.packets.front().rate;}
        buffered=std::clamp(seconds,0.,30.);
    }
    void run(){
        output::ptr out;size_t active=0;unsigned targetRate=0;Resampler<audio_sample> resampler;GUID activeBackend={},activeDevice={};
        while(true){
            GUID b={},d={};size_t generation=0;bool on=false;
            {std::unique_lock<std::mutex> lock(mutex);
             auto wake=[&]{return stopped||revision!=active||!queue.packets.empty();};
             if(enabled&&out.is_valid())changed.wait_for(lock,std::chrono::milliseconds(5),wake);else changed.wait(lock,wake);
             if(stopped)break;generation=revision;on=enabled;b=backend;d=device;}
            if(generation!=active){
                // Seeking within the same cue device only flushes it: do not
                // reconnect Bluetooth/open a driver on every waveform click.
                if(out.is_valid()&&on&&b==activeBackend&&d==activeDevice){try{out->flush();}catch(...){out.release();}}
                else out.release();
                buffered=0;resampler.reset();active=generation;activeBackend=b;activeDevice=d;
            }
            if(!on)continue;
            try{
                bool empty;{std::lock_guard<std::mutex> lock(mutex);empty=queue.packets.empty();}
                if(empty){if(out.is_valid()){bool ready=false;out->update(ready);measure(out);}continue;}
                if(!out.is_valid()){
                    auto core=output_manager::get()->getCoreConfig();
                    if(core.m_output==b&&core.m_device==d)throw std::runtime_error("Choose a headphone device different from the master output");
                    auto entry=output_entry::g_find(b);entry->instantiate(out,d,0.1,false,entry->default_bit_depth(d));
                    targetRate=out->get_forced_sample_rate_();
                    if(targetRate&&(targetRate<1000||targetRate>768000))throw std::runtime_error("Unsupported headphone sample rate");
                }
                bool ready=false;out->update(ready);
                if(!ready){std::unique_lock<std::mutex> lock(mutex);changed.wait_for(lock,std::chrono::milliseconds(2));continue;}
                Packet<audio_sample> packet;bool reset=false;
                {std::lock_guard<std::mutex> lock(mutex);if(revision!=generation||queue.packets.empty())continue;
                 reset=queue.discontinuity;queue.discontinuity=false;packet=queue.pop();}
                if(reset){out->flush();resampler.reset();out->update(ready);if(!ready)continue;}
                auto samples=resampler.process(packet.audio,packet.rate,targetRate?targetRate:packet.rate);
                if(!samples.empty()){audio_chunk_impl chunk;chunk.set_data(samples.data(),samples.size()/2,2,targetRate?targetRate:packet.rate,audio_chunk::channel_config_stereo);out->process_samples(chunk);}
                measure(out);
            }catch(const std::exception& e){
                out.release();buffered=0;resampler.reset();std::lock_guard<std::mutex> lock(mutex);
                if(revision==generation){error=e.what();enabled=false;queue.clear();}
            }catch(...){
                out.release();buffered=0;resampler.reset();std::lock_guard<std::mutex> lock(mutex);
                if(revision==generation){error="Unknown headphone output error";enabled=false;queue.clear();}
            }
        }
    }
public:
    ~Sink(){stop();}
    size_t configure(bool on,const GUID& b,const GUID& d){
        size_t token;
        {std::lock_guard<std::mutex> lock(mutex);enabled=on;backend=b;device=d;queue.clear();error.clear();token=++revision;serial=token;}
        if(on&&!worker.joinable())worker=std::thread([this]{run();});changed.notify_all();
        return token;
    }
    // Called by the master thread: a contended control operation just drops this
    // cue block. No driver wait and no growth beyond the bounded queue.
    bool push(std::vector<audio_sample> samples,unsigned rate,size_t token=SIZE_MAX){
        std::unique_lock<std::mutex> lock(mutex,std::try_to_lock);if(!lock||!enabled||(token!=SIZE_MAX&&token!=revision))return false;
        queue.push({std::move(samples),rate});changed.notify_one();return true;
    }
    size_t token()const{return serial.load();}
    double latency()const{return buffered.load();}
    std::string lastError(){std::lock_guard<std::mutex> lock(mutex);return error;}
    void stop(){
        {std::lock_guard<std::mutex> lock(mutex);stopped=true;enabled=false;queue.clear();}
        changed.notify_all();if(worker.joinable())worker.join();
    }
};
}
