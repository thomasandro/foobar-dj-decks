#pragma once
#include <cmath>
#include <algorithm>
namespace dj_tempo {
enum class Match { ok, missing, outside };
inline double range(double percent) { return percent==8 || percent==50 ? percent/100.0 : 0.16; }
inline double clamp(double speed,double percent) {
    const double r=range(percent);
    return (std::max)(1-r,(std::min)(1+r,speed));
}
inline Match match(double ownBpm, double otherBpm, double otherSpeed, double & speed, double percent=16, double* factor=nullptr) {
    if (!std::isfinite(ownBpm) || !std::isfinite(otherBpm) || !std::isfinite(otherSpeed) ||
        ownBpm <= 0 || otherBpm <= 0 || otherSpeed <= 0) return Match::missing;
    const double r=range(percent);
    const double current=std::isfinite(speed)&&speed>0?speed:1.0;
    bool found=false;double best=0,bestFactor=1,distance=0;
    // Exact first gives it deterministic preference when adjustments tie.
    for(double candidate:{1.0,0.5,2.0}){
        const double required=(otherBpm/ownBpm)*otherSpeed*candidate;
        if(!std::isfinite(required)||required<1-r-1e-9||required>1+r+1e-9)continue;
        double fit=clamp(required,percent),change=std::abs(fit-current);
        if(!found||change<distance-1e-12){found=true;best=fit;bestFactor=candidate;distance=change;}
    }
    if(!found)return Match::outside;
    speed=best;if(factor)*factor=bestFactor;
    return Match::ok;
}
}
