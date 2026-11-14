#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include "picojson.h"
#pragma comment(lib,"bcrypt.lib")
namespace dj_stems {
inline constexpr const char* engine="demucs-4.0.1-htdemucs-v1";
inline const wchar_t* names[4]={L"drums.f32",L"bass.f32",L"other.f32",L"vocals.f32"};
inline std::wstring localRoot(){
    wchar_t path[32768]={};DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,32768);
    if(!n||n>=32768)throw std::runtime_error("LOCALAPPDATA unavailable");
    return std::wstring(path)+L"\\DJDecks";
}
inline std::string key(const std::wstring& native,unsigned subsong){
    WIN32_FILE_ATTRIBUTE_DATA data={};
    if(!GetFileAttributesExW(native.c_str(),GetFileExInfoStandard,&data)||(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))return {};
    std::wstring identity=native+L"|"+std::to_wstring(subsong)+L"|"+std::to_wstring(data.nFileSizeHigh)+L":"+std::to_wstring(data.nFileSizeLow)+L"|"+std::to_wstring(data.ftLastWriteTime.dwHighDateTime)+L":"+std::to_wstring(data.ftLastWriteTime.dwLowDateTime)+L"|demucs-4.0.1-htdemucs-v1";
    BCRYPT_ALG_HANDLE alg=nullptr;unsigned char digest[32]={};
    if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)throw std::runtime_error("SHA256 unavailable");
    DWORD size=0,returned=0;BCRYPT_HASH_HANDLE hash=nullptr;
    auto status=BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,(PUCHAR)&size,sizeof(size),&returned,0);
    std::vector<unsigned char> object(size);
    if(status>=0)status=BCryptCreateHash(alg,&hash,object.data(),size,nullptr,0,0);
    if(status>=0)status=BCryptHashData(hash,(PUCHAR)identity.data(),(ULONG)(identity.size()*sizeof(wchar_t)),0);
    if(status>=0)status=BCryptFinishHash(hash,digest,32,0);
    if(hash)BCryptDestroyHash(hash);BCryptCloseAlgorithmProvider(alg,0);if(status<0)throw std::runtime_error("Cache hash failed");
    const char* hex="0123456789abcdef";std::string result;for(auto c:digest){result+=hex[c>>4];result+=hex[c&15];}return result;
}
inline picojson::value readJson(const std::wstring& path,size_t limit=65536){
    picojson::value empty(picojson::object{});
    std::ifstream in(path,std::ios::binary);if(!in)return empty;
    in.seekg(0,std::ios::end);auto size=in.tellg();if(size<=0||size>(std::streamoff)limit)return empty;
    std::string s((size_t)size,'\0');in.seekg(0);in.read(s.data(),s.size());if(!in)return empty;
    picojson::value value;if(!picojson::parse(value,s).empty()||!value.is<picojson::object>())return empty;return value;
}
struct WaveWriter {
    std::ofstream out;unsigned rate;uint32_t bytes=0;
    WaveWriter(const std::wstring& file,unsigned r):out(file,std::ios::binary),rate(r){
        if(!out||!r||r>384000)throw std::runtime_error("Cannot export audio");
        char blank[44]={};out.write(blank,44);
    }
    template<class T>void append(const T* data,size_t frames,unsigned channels){
        if(!channels||bytes+uint64_t(frames)*8>1024ull*1024*1024||(bytes/8+frames)>uint64_t(rate)*1800)throw std::runtime_error("Stem input exceeds 30 minutes or 1 GiB");
        std::vector<float> stereo;stereo.reserve(frames*2);
        for(size_t n=0;n<frames;++n)for(int ch=0;ch<2;++ch){float v=(float)data[n*channels+(channels>1?ch:0)];if(!std::isfinite(v))throw std::runtime_error("Invalid decoded sample");stereo.push_back(v);}
        out.write((const char*)stereo.data(),stereo.size()*4);bytes+=(uint32_t)(stereo.size()*4);if(!out)throw std::runtime_error("Audio export failed (disk full?)");
    }
    void finish(){
        out.seekp(0);auto u16=[&](uint16_t x){out.write((const char*)&x,2);};auto u32=[&](uint32_t x){out.write((const char*)&x,4);};
        out.write("RIFF",4);u32(bytes+36);out.write("WAVEfmt ",8);u32(16);u16(3);u16(2);u32(rate);u32(rate*8);u16(8);u16(32);out.write("data",4);u32(bytes);out.flush();if(!out||!bytes)throw std::runtime_error("Empty or failed audio export");out.close();
    }
};
struct Mapping {
    HANDLE file=INVALID_HANDLE_VALUE,map=nullptr;const float* samples=nullptr;
    Mapping()=default;Mapping(const Mapping&)=delete;Mapping& operator=(const Mapping&)=delete;
    ~Mapping(){if(samples)UnmapViewOfFile(samples);if(map)CloseHandle(map);if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);}
    void open(const std::wstring& path,uint64_t bytes){
        file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        LARGE_INTEGER size={};if(file==INVALID_HANDLE_VALUE||!GetFileSizeEx(file,&size)||(uint64_t)size.QuadPart!=bytes)throw std::runtime_error("Invalid stem file size");
        map=CreateFileMappingW(file,nullptr,PAGE_READONLY,0,0,nullptr);if(!map)throw std::runtime_error("Stem mapping failed");
        samples=(const float*)MapViewOfFile(map,FILE_MAP_READ,0,0,0);if(!samples)throw std::runtime_error("Stem mapping view failed");
    }
};
struct Reader {
    std::array<Mapping,4> files;uint64_t frames=0,position=0;unsigned rate=44100,mask=15;
    float gain[4]={1,1,1,1};
    float volume[4]={1,1,1,1};unsigned solo=0;
    float target(int i)const{
        // Solo temporarily overrides mute, preserving the original mute mask.
        bool audible=solo?(solo&(1<<i))!=0:(mask&(1<<i))!=0;
        float v=std::isfinite(volume[i])?(std::max)(0.f,(std::min)(1.f,volume[i])):0.f;
        return audible?v:0.f;
    }
    Reader(const std::wstring& folder,const std::string& expected){
        auto m=readJson(folder+L"\\manifest.json");
        if(!m.is<picojson::object>()||!m.get("engine").is<std::string>()||m.get("engine").get<std::string>()!=engine||!m.get("key").is<std::string>()||m.get("key").get<std::string>()!=expected)throw std::runtime_error("Invalid stem cache identity");
        auto& n=m.get("frames");auto& r=m.get("rate");auto& c=m.get("channels");
        if(!n.is<double>()||!std::isfinite(n.get<double>())||n.get<double>()<1||n.get<double>()>44100.0*1800||std::floor(n.get<double>())!=n.get<double>()||!r.is<double>()||r.get<double>()!=44100||!c.is<double>()||c.get<double>()!=2)throw std::runtime_error("Invalid stem cache format");
        frames=(uint64_t)n.get<double>();for(int i=0;i<4;++i)files[i].open(folder+L"\\"+names[i],frames*8);
    }
    void seek(double seconds){position=(uint64_t)((std::max)(0.0,(std::min)(double(frames),seconds*rate)));for(int i=0;i<4;++i)gain[i]=target(i);}
    template<class T>size_t read(std::vector<T>& out,size_t count=2048){
        size_t n=(size_t)(std::min)((uint64_t)count,frames-position);out.reserve(out.size()+n*2);
        for(size_t f=0;f<n;++f){double sum[2]={};
            for(int i=0;i<4;++i){float desired=target(i);float step=1.f/(rate*.005f);gain[i]+= (std::max)(-step,(std::min)(step,desired-gain[i]));
                for(int ch=0;ch<2;++ch){float s=files[i].samples[(position+f)*2+ch];if(!std::isfinite(s))throw std::runtime_error("Invalid sample in stem cache");sum[ch]+=s*gain[i];}}
            out.push_back((T)sum[0]);out.push_back((T)sum[1]);
        }position+=n;return n;
    }
};
inline std::wstring cacheRoot(){return localRoot()+L"\\stems-cache\\v1";}
inline std::wstring folder(const std::string& key){return cacheRoot()+L"\\"+std::wstring(key.begin(),key.end());}
}
