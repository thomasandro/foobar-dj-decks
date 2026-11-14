
#define NOMINMAX
#include "../sdk/foobar2000/foo_dj/stem_cache.h"
#include "../sdk/foobar2000/foo_dj/third_party/soundtouch/include/SoundTouch.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <iostream>
#include <filesystem>
using audio_sample=double;
struct abort_callback { void check() {} };
struct audio_chunk_impl { std::vector<audio_sample> data; unsigned get_channel_count(){return 2;} unsigned get_sample_rate(){return 48000;} };
struct Input { unsigned pos=0; bool run(audio_chunk_impl & c,abort_callback &){if(pos>=144000)return false;unsigned n=(std::min)(2048u,144000-pos); c.data.resize(n*2);for(unsigned i=0;i<n;i++){float x=(float)(0.5*sin(2*3.141592653589793*440*(pos+i)/48000));c.data[2*i]=c.data[2*i+1]=x;}pos+=n;return true;} };
struct Deck { std::shared_ptr<dj_stems::Reader> stems;bool playing=true; double framesPlayed=0; unsigned rate=48000,channels=2;std::vector<audio_sample> src,pending;double srcPos=0;size_t pendingPos=0;bool eof=false,stretchFlushed=false;std::shared_ptr<soundtouch::SoundTouch> stretch;std::shared_ptr<Input> helper=std::make_shared<Input>();size_t pendingFrames(){return(pending.size()-pendingPos)/2;} };
void AppendStereo(Deck &d,audio_chunk_impl &c){d.src.insert(d.src.end(),c.data.begin(),c.data.end());}
