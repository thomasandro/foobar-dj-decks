#define NOMINMAX
#include "../sdk/foobar2000/foo_dj/stem_cache.h"
#include <filesystem>
#include <cassert>
#include <iostream>
int main(){
    std::wstring folder=L"build\\tests\\stem-cache";std::filesystem::create_directories(folder);
    std::string id(64,'a');picojson::object m;m["engine"]=picojson::value(std::string(dj_stems::engine));m["key"]=picojson::value(id);m["rate"]=picojson::value(44100.);m["channels"]=picojson::value(2.);m["frames"]=picojson::value(1000.);
    {std::ofstream out(folder+L"\\manifest.json");out<<picojson::value(m).serialize();}
    for(int k=0;k<4;++k){std::vector<float> data;for(int n=0;n<1000;++n){data.push_back((k+1)*.1f);data.push_back(-(k+1)*.1f);}std::ofstream out(folder+L"\\"+dj_stems::names[k],std::ios::binary);out.write((const char*)data.data(),data.size()*4);}
    for(unsigned mask=0;mask<16;++mask){dj_stems::Reader r(folder,id);r.mask=mask;r.seek(0);std::vector<double> samples;assert(r.read(samples,100)==100);double expected=0;for(int k=0;k<4;++k)if(mask&(1<<k))expected+=(k+1)*.1;
        assert(std::abs(samples[40]-expected)<1e-6&&std::abs(samples[41]+expected)<1e-6);r.seek(100);samples.clear();assert(r.read(samples)==0);}
    {dj_stems::Reader a(folder,id),b(folder,id);a.seek(.01);assert(a.position!=b.position);a.mask=0;std::vector<double> samples;a.read(samples,500);assert(std::abs(samples.back())<1e-6&&b.position==0);}
    for(unsigned solo=0;solo<16;++solo)for(unsigned mask=0;mask<16;++mask){
        dj_stems::Reader r(folder,id);r.mask=mask;r.solo=solo;float levels[]={0,.25f,.5f,1};double expected=0;
        for(int k=0;k<4;++k){r.volume[k]=levels[k];if((solo?solo:mask)&(1<<k))expected+=(k+1)*.1*levels[k];}
        r.seek(0);std::vector<double> samples;r.read(samples,10);assert(std::abs(samples[0]-expected)<1e-6&&std::abs(samples[1]+expected)<1e-6);
    }
    {dj_stems::Reader a(folder,id),b(folder,id);a.mask=0;a.solo=2;a.volume[1]=.5f;a.seek(0);std::vector<double> samples;a.read(samples,1);assert(std::abs(samples[0]-.1)<1e-6);assert(b.solo==0&&b.mask==15&&b.volume[1]==1);
     a.solo=0;samples.clear();a.read(samples,500);assert(std::abs(samples.back())<1e-6); // original mutes restored
     a.mask=15;a.volume[0]=NAN;a.volume[1]=-2;a.volume[2]=5;assert(a.target(0)==0&&a.target(1)==0&&a.target(2)==1);}
    bool bad=false;try{dj_stems::Reader r(folder,std::string(64,'b'));}catch(...){bad=true;}assert(bad);
    m["frames"]=picojson::value(-1.);{std::ofstream out(folder+L"\\manifest.json");out<<picojson::value(m).serialize();}
    bad=false;try{dj_stems::Reader r(folder,id);}catch(...){bad=true;}assert(bad);
    std::wstring wave=folder+L"\\export.wav";
    {dj_stems::WaveWriter w(wave,44100);float x[]={.1f,.2f,.3f};w.append(x,3,1);w.finish();}
    assert(std::filesystem::file_size(wave)==44+3*8);auto k0=dj_stems::key(wave,0);assert(k0.size()==64&&k0!=dj_stems::key(wave,1));
    {std::ofstream out(wave,std::ios::app|std::ios::binary);out.put('x');}assert(dj_stems::key(wave,0)!=k0);
    assert(dj_stems::readJson(folder+L"\\missing.json").is<picojson::object>());
    std::cout<<"Stem cache: 16 masks, stereo sum, ramp, independent clocks, EOF/seek, invalid identity/shape, export and source/subsong invalidation passed\n";
}
