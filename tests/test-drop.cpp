#include "../sdk/foobar2000/foo_dj/webview_drop.h"
#include <cassert>
#include <iostream>
int main() {
    std::string path; unsigned subsong = 0;
    const std::string item = R"({"path":"C:\\Music\\été 🎵.cue","subsong":7})";
    for (const std::string type : {"foobar-playlist-tracks-v1", "foobar-browser-tracks-v1"}) {
        assert(dj_drop::parse(type + "\n{\"type\":\"" + type + "\",\"items\":[" + item + "]}", path, subsong));
        assert(path == "C:\\Music\\été 🎵.cue" && subsong == 7);
    }
    for (const std::string bad : {"hello", "C:\\song.mp3", "foobar-playlist-tracks-v1\n{}",
        "foobar-browser-tracks-v1\n{\"type\":\"foobar-playlist-tracks-v1\",\"items\":[]}"}) {
        assert(!dj_drop::parse(bad, path, subsong));
    }
    for (const std::string index : {"-1", "1.5", "4294967296", "null", "\"0\""}) {
        assert(!dj_drop::parse("foobar-browser-tracks-v1\n{\"type\":\"foobar-browser-tracks-v1\",\"items\":[{\"path\":\"x\",\"subsong\":" + index + "}]}", path, subsong));
    }
    assert(!dj_drop::parse("foobar-browser-tracks-v1\n{\"type\":\"foobar-browser-tracks-v1\",\"items\":[{\"path\":\"x\\u0000y\",\"subsong\":0}]}", path, subsong));
    std::cout << "Drop parser tests passed\n";
}
