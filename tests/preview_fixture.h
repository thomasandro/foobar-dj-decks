#define NOMINMAX
#include <windows.h>
#include <atomic>
#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/preview_plan.h"
#include "../sdk/foobar2000/foo_dj/preview_visual.h"
#include "../sdk/foobar2000/foo_dj/preview_overview.h"
using audio_sample=double;
struct abort_callback_impl{bool cancelled=false;int sleeps=0;void check(){if(cancelled)throw std::runtime_error("cancelled");}void sleep(double){check();++sleeps;}};
struct Native{bool empty=false;bool is_empty()const{return empty;}};
struct filesystem{static Native g_get_native_path(const char* path){return {std::string(path).find("http:")==0};}};
struct file{};template<class T>using service_ptr_t=std::shared_ptr<T>;
constexpr int input_flag_no_looping=1;
struct Location{std::string path;unsigned subsong;};
inline Location make_playable_location(const char* path,unsigned subsong){return {path,subsong};}
namespace fake_preview{inline double length=100;inline bool seekable=true;inline unsigned subsong=0;inline int alive=0,opened=0;}
struct file_info_impl{double get_length()const{return fake_preview::length;}};
struct audio_chunk_impl{std::vector<double> data;unsigned get_sample_rate()const{return 1000;}unsigned get_channel_count()const{return 2;}const double* get_data()const{return data.data();}size_t get_sample_count()const{return data.size()/2;}};
struct input_helper{
    size_t position=0;input_helper(){++fake_preview::alive;++fake_preview::opened;}~input_helper(){--fake_preview::alive;}
    void open(service_ptr_t<file>,Location loc,int,abort_callback_impl& abort){abort.check();fake_preview::subsong=loc.subsong;}
    void get_info(unsigned,file_info_impl&,abort_callback_impl& abort){abort.check();}
    bool can_seek()const{return fake_preview::seekable;}
    void seek(double seconds,abort_callback_impl& abort){abort.check();position=(size_t)std::llround(seconds*1000);}
    bool run(audio_chunk_impl& chunk,abort_callback_impl& abort){abort.check();size_t end=fake_preview::length>0?(size_t)(fake_preview::length*1000):100000;
        if(position>=end)return false;size_t count=(std::min)(size_t(1024),end-position);chunk.data.clear();
        for(size_t n=0;n<count;++n){chunk.data.push_back((double)(position+n+1));chunk.data.push_back(-(double)(position+n+1));}position+=count;return true;}
};
namespace dj_queue{struct Item{std::string path;unsigned subsong=0;};}
struct CDjPanel{
    struct PreviewRequest{dj_queue::Item item;int mode=0;size_t token=0;double position=-1;};
    struct Sink{std::vector<double> data;size_t generation=1;bool fail=false;
        bool push(const std::vector<double>& samples,unsigned rate,size_t token){assert(rate==1000);if(fail||generation!=token)return false;data.insert(data.end(),samples.begin(),samples.end());return true;}
        size_t token()const{return generation;}std::string lastError()const{return fail?"headphones unavailable":"";}
    }m_cueOutput;
    std::atomic<double> m_cueVolume{1.};
    dj_preview::Visual m_previewVisual;
    dj_preview::OverviewCache m_previewOverviewCache;
    std::string PreviewOverviewKey(const dj_queue::Item& item){return item.path+std::to_string(item.subsong);}
    void ScanPreviewOverview(const PreviewRequest&,abort_callback_impl&);
    void PlayPreview(const PreviewRequest&,abort_callback_impl&);
};
