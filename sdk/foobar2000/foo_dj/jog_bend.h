#pragma once
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace dj_jog {
// One coherent publication for the UI writer and audio reader (also on x86).
// Age is evaluated by the audio thread: a stalled UI cannot leave bend engaged.
class Bend {
    std::atomic<std::uint64_t> request{0};
    static constexpr std::uint64_t mask=(std::uint64_t(1)<<48)-1;
    static double decode(std::uint64_t data,std::uint64_t now) {
        auto age=((now&mask)-(data>>16))&mask;
        if(age>=300)return 0;
        int amplitude=int(data&65535);if(amplitude>=32768)amplitude-=65536;
        return amplitude/10000. * (age<=120?1.:(300.-age)/180.);
    }
public:
    void clear(){request.store(0,std::memory_order_release);}
    double value(std::uint64_t now)const{return decode(request.load(std::memory_order_acquire),now);}
    void pulse(double ticks,int strength,std::uint64_t now){
        if(!std::isfinite(ticks)||ticks==0)return;
        constexpr int limits[]={50,100,200,400,800};
        const int limit=limits[std::clamp(strength,0,4)];
        auto old=request.load(std::memory_order_acquire);
        int amount=int(std::lround(std::clamp(decode(old,now)*10000.+std::clamp(ticks,-16.,16.)*limit/4.,-double(limit),double(limit))));
        request.store(((now&mask)<<16)|std::uint16_t(amount),std::memory_order_release);
    }
};
}
