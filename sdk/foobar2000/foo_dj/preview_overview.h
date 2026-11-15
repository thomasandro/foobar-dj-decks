#pragma once
#include "preview_visual.h"
#include <deque>
namespace dj_preview {
struct Overview {
    double length=0;std::array<float,32> peaks{};std::array<bool,32> known{};
    bool complete()const{for(bool item:known)if(!item)return false;return true;}
};
// Only the low-priority overview worker accesses this bounded, in-memory cache.
class OverviewCache {
    std::deque<std::pair<std::string,Overview>> entries;
public:
    Overview get(const std::string& key){for(auto& item:entries)if(item.first==key)return item.second;return {};}
    void put(std::string key,const Overview& value){
        for(auto it=entries.begin();it!=entries.end();++it)if(it->first==key){entries.erase(it);break;}
        entries.emplace_front(std::move(key),value);while(entries.size()>8)entries.pop_back();
    }
};
inline std::vector<size_t> overviewOrder(){
    std::vector<size_t> order{0,31};for(unsigned n=0;n<32;++n){unsigned index=0;for(unsigned bit=0;bit<5;++bit)index=(index<<1)|((n>>bit)&1);if(index!=0&&index!=31)order.push_back(index);}return order;
}
}
