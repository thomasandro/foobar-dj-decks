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
inline Match match(double ownBpm, double otherBpm, double otherSpeed, double & speed, double percent=16) {
    if (!std::isfinite(ownBpm) || !std::isfinite(otherBpm) || !std::isfinite(otherSpeed) ||
        ownBpm <= 0 || otherBpm <= 0 || otherSpeed <= 0) return Match::missing;
    const double required = otherBpm * otherSpeed / ownBpm;
    const double r=range(percent);
    if (!std::isfinite(required) || required < 1-r - 1e-9 || required > 1+r + 1e-9) return Match::outside;
    speed = clamp(required,percent);
    return Match::ok;
}
}
