#include "../sdk/foobar2000/foo_dj/deck_queue.h"
#include <cassert>
#include <iostream>
using namespace dj_queue;
int main(){
    std::vector<Item> q={{"file://école.flac",2},{"b",0},{"b",0},{"d",7},{"e",0}};
    auto restored=decode(encode(q));assert(restored.size()==5&&restored[0].path==q[0].path&&restored[0].subsong==2);
    assert(decode(encode({})).empty());
    auto selected=move(q,{3,1,3,999},5);
    assert((selected==std::vector<size_t>{3,4}));
    assert(q[0].path=="file://école.flac"&&q[1].path=="b"&&q[2].path=="e"&&q[3].path=="b"&&q[4].path=="d");
    move(q,{3,4},0);assert(q[0].path=="b"&&q[1].path=="d");
    auto copy=q;move(q,{0,1},1);for(size_t n=0;n<q.size();++n)assert(q[n].path==copy[n].path);
    erase(q,{4,0,999,0});assert(q.size()==3&&q[0].path=="d");
    picojson::value malformed;
    assert(picojson::parse(malformed,R"([{"path":"a","subsong":-1},{"path":"a","subsong":1.5},{"path":"a","subsong":4294967296},{"path":"a\u0000b","subsong":0},{"path":"","subsong":0},{"path":"ok","subsong":4294967295}])").empty());
    auto validItems=decode(malformed);assert(validItems.size()==1&&validItems[0].subsong==4294967295u);
    // All insertion boundaries and every multi-selection: no loss/duplication/order inversion.
    for(unsigned mask=0;mask<32;++mask)for(size_t before=0;before<=5;++before){
        std::vector<Item> all;std::vector<size_t> sel;for(unsigned n=0;n<5;++n){all.push_back({std::to_string(n),n});if(mask&(1<<n))sel.push_back(n);}
        auto moved=move(all,sel,before);assert(all.size()==5&&moved.size()==sel.size());
        std::vector<unsigned> ids;for(auto& i:all)ids.push_back(i.subsong);auto sorted=ids;std::sort(sorted.begin(),sorted.end());assert((sorted==std::vector<unsigned>{0,1,2,3,4}));
        for(size_t n=0;n<moved.size();++n)assert(all[moved[n]].subsong==sel[n]);
    }
    std::cout<<"Queue: round-trip, malformed entries, duplicates, deletion and 192 reorder cases passed\n";
}
