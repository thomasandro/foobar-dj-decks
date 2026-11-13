#pragma once
#include <cmath>
#include <algorithm>
namespace dj_tempo {
enum class Match { ok, missing, outside };
inline Match match(double ownBpm, double otherBpm, double otherSpeed, double & speed) {
    if (!std::isfinite(ownBpm) || !std::isfinite(otherBpm) || !std::isfinite(otherSpeed) ||
        ownBpm <= 0 || otherBpm <= 0 || otherSpeed <= 0) return Match::missing;
    const double required = otherBpm * otherSpeed / ownBpm;
    if (!std::isfinite(required) || required < 0.84 - 1e-9 || required > 1.16 + 1e-9) return Match::outside;
    speed = (std::max)(0.84, (std::min)(1.16, required));
    return Match::ok;
}
}
