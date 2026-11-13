#pragma once
#include "picojson.h"
#include <string>
#include <cmath>

namespace dj_drop {
// CF_UNICODETEXT fallback exported by WebView2. Never interpret arbitrary text as a path.
inline bool parse(const std::string & text, std::string & path, unsigned & subsong) {
    const std::string playlist = "foobar-playlist-tracks-v1\n";
    const std::string browser = "foobar-browser-tracks-v1\n";
    const std::string * prefix = nullptr;
    if (text.compare(0, playlist.size(), playlist) == 0) prefix = &playlist;
    else if (text.compare(0, browser.size(), browser) == 0) prefix = &browser;
    if (!prefix || text.size() > 8 * 1024 * 1024) return false;
    picojson::value root;
    if (!picojson::parse(root, text.substr(prefix->size())).empty() || !root.is<picojson::object>()) return false;
    const auto & type = root.get("type");
    if (!type.is<std::string>() || type.get<std::string>() != prefix->substr(0, prefix->size() - 1)) return false;
    const auto & items = root.get("items");
    if (!items.is<picojson::array>() || items.get<picojson::array>().empty()) return false;
    const auto & first = items.get<picojson::array>()[0];
    if (!first.is<picojson::object>()) return false;
    const auto & p = first.get("path");
    const auto & s = first.get("subsong");
    if (!p.is<std::string>() || !s.is<double>()) return false;
    const std::string candidate = p.get<std::string>();
    const double index = s.get<double>();
    if (candidate.empty() || candidate.size() > 32768 || candidate.find('\0') != std::string::npos ||
        candidate.find('\r') != std::string::npos || candidate.find('\n') != std::string::npos ||
        !std::isfinite(index) || index < 0 || index > 4294967295.0 || std::floor(index) != index) return false;
    path = candidate;
    subsong = static_cast<unsigned>(index);
    return true;
}
}
