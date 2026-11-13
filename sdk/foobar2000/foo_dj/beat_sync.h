#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
namespace dj_beats {
struct Grid { double period = 0, offset = 0, confidence = 0; bool valid = false; };
inline Grid manual(double bpm, double anchor) {
    Grid out;
    if (!std::isfinite(bpm) || bpm < 40 || bpm > 250 || !std::isfinite(anchor)) return out;
    out.period = 60.0 / bpm;
    out.offset = std::fmod(anchor, out.period);
    if (out.offset < 0) out.offset += out.period;
    out.confidence = 1; out.valid = true;
    return out;
}
// Local onset phase at the tagged quarter-note tempo. This is not downbeat detection.
inline Grid fit(const std::vector<float> & rms, double bpm, double position) {
    Grid grid;
    if (!std::isfinite(bpm) || bpm < 40 || bpm > 250 || !std::isfinite(position) || position < 0) return grid;
    grid.period = 60.0 / bpm;
    const size_t first = (size_t)((std::max)(0.01, position - 12.0) * 100.0);
    const size_t last = (std::min)(rms.size(), (size_t)((position + 12.0) * 100.0));
    if (last <= first || (last-first) * 0.01 < 4 * grid.period) return grid;
    double maxRise = 0;
    for (size_t i = first; i < last; ++i) {
        if (!std::isfinite(rms[i]) || !std::isfinite(rms[i-1])) continue;
        maxRise = (std::max)(maxRise, (double)rms[i] - rms[i-1]);
    }
    if (maxRise < 0.0001) return grid;
    constexpr double tau = 6.2831853071795864769;
    double x = 0, y = 0, total = 0; size_t attacks = 0;
    for (size_t i = first; i < last; ++i) {
        const double rise = (double)rms[i] - rms[i-1];
        if (!std::isfinite(rise) || rise < maxRise * 0.15) continue;
        const double weight = rise * rise;
        const double phase = tau * (i * 0.01) / grid.period;
        x += weight * std::cos(phase); y += weight * std::sin(phase); total += weight; ++attacks;
    }
    if (attacks < 6 || total <= 0) return grid;
    grid.confidence = std::sqrt(x*x+y*y) / total;
    double angle = std::atan2(y,x); if (angle < 0) angle += tau;
    grid.offset = angle / tau * grid.period;
    grid.valid = grid.confidence >= 0.60;
    return grid;
}
inline double fraction(double x) { return x - std::floor(x); }
inline bool align(const Grid & own, const Grid & reference, double ownPosition, double refPosition, double length, double & target) {
    if (!own.valid || !reference.valid || own.period <= 0 || reference.period <= 0 || length <= 0) return false;
    const double phase = fraction((refPosition-reference.offset) / reference.period);
    const double beat = std::round((ownPosition-own.offset) / own.period - phase);
    target = own.offset + (beat+phase) * own.period;
    if (target < 0) target += own.period;
    if (target > length-0.1) target -= own.period;
    return std::isfinite(target) && target >= 0 && target <= length-0.1 && std::abs(target-ownPosition) <= own.period + 0.01;
}
struct Ticks { std::vector<int> pixels; unsigned stride = 1; };
inline Ticks ticks(const Grid & grid, double start, double span, int width) {
    Ticks out;
    if (!grid.valid || !std::isfinite(grid.period) || grid.period <= 0 || !std::isfinite(grid.offset) ||
        !std::isfinite(start) || !std::isfinite(span) || span <= 0 || width <= 0) return out;
    while (grid.period * width / span * out.stride < 8 && out.stride < 1048576) out.stride *= 2;
    const double step = grid.period * out.stride;
    const double first = std::ceil((start-grid.offset) / step);
    for (int n = 0; n < width / 8 + 3; ++n) {
        const double t = grid.offset + (first+n) * step;
        if (t >= start+span) break;
        const int x = (int)std::floor((t-start) / span * width);
        if (x >= 0 && x < width) out.pixels.push_back(x);
    }
    return out;
}
}
