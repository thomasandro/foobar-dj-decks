#define NOMINMAX
#include <windows.h>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <cassert>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/silence_trim.h"
#include "../sdk/foobar2000/foo_dj/preview_plan.h"
#include "../sdk/foobar2000/foo_dj/deck_queue.h"
using audio_sample=double;using t_uint32=unsigned;
struct abort_callback_impl{std::atomic<bool> cancelled{false};void set(){cancelled=true;}bool is_aborting()const{return cancelled.load();}void check(){if(cancelled)throw std::runtime_error("abort");}};
struct file{};template<class T>using service_ptr_t=std::shared_ptr<T>;constexpr int input_flag_no_looping=1;
struct Location{std::string path;unsigned subsong;};inline Location make_playable_location(const char* p,unsigned s){return {p,s};}
struct file_info_impl{double get_length()const{return 10;}};
struct audio_chunk_impl{std::vector<double> data;const double* get_data()const{return data.data();}size_t get_sample_count()const{return data.size()/2;}unsigned get_sample_rate()const{return 1000;}unsigned get_channel_count()const{return 2;}};
struct input_helper{size_t position=0;void open(service_ptr_t<file>,Location location,int,abort_callback_impl& abort){abort.check();if(location.path=="bad")throw std::runtime_error("decode failed");if(location.path=="nonstd")throw 1;}
    void get_info(unsigned,file_info_impl&,abort_callback_impl& abort){abort.check();}bool can_seek()const{return true;}
    void seek(double seconds,abort_callback_impl& abort){abort.check();position=(size_t)std::llround(seconds*1000);}
    bool run(audio_chunk_impl& chunk,abort_callback_impl& abort){abort.check();if(position>=10000)return false;auto count=(std::min)(size_t(256),10000-position);chunk.data.clear();
        for(size_t n=0;n<count;++n){auto time=position+n;double sample=time>=2000&&time<7000?.2:0;chunk.data.push_back(sample);chunk.data.push_back(-sample);}position+=count;return true;}
};
struct Deck{std::shared_ptr<input_helper> helper;std::vector<audio_sample> src;double length=0,trimStart=0,trimEnd=0,framesPlayed=0;unsigned rate=0,channels=0,sourceRate=0,sourceChannels=0;bool eof=false,loaded=false,playing=false;};
inline void AppendStereo(Deck& deck,const audio_chunk_impl& chunk){deck.src.insert(deck.src.end(),chunk.data.begin(),chunk.data.end());}
inline dj_silence::Options options{true,true,-60};inline dj_silence::Options TrimOptions(){return options;}
inline std::atomic<bool> g_english{false};
