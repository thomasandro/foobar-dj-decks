#pragma once
#include "picojson.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

namespace dj_queue {
struct Item { std::string path; unsigned subsong=0; };
inline bool valid(const Item& i) {
    return !i.path.empty() && i.path.size()<=32768 && i.path.find_first_of("\r\n") == std::string::npos && i.path.find('\0')==std::string::npos;
}
// Preserve source order, duplicates and subsongs. Never silently truncate.
inline bool alternate(const std::vector<Item>& source,std::vector<Item>& a,std::vector<Item>& b){
    if(source.empty()||source.size()>20000)return false;
    std::vector<Item> next[2];
    for(size_t n=0;n<source.size();++n){if(!valid(source[n]))return false;next[n%2].push_back(source[n]);}
    a.swap(next[0]);b.swap(next[1]);return true;
}
inline picojson::value encode(const std::vector<Item>& items) {
    picojson::array out;
    for(auto& i:items) { picojson::object o; o["path"]=picojson::value(i.path);o["subsong"]=picojson::value(double(i.subsong));out.emplace_back(o); }
    return picojson::value(out);
}
inline std::vector<Item> decode(const picojson::value& value,size_t limit=10000) {
    std::vector<Item> out;
    if(!value.is<picojson::array>())return out;
    for(auto& v:value.get<picojson::array>()) {
        if(out.size()>=limit)break;
        if(!v.is<picojson::object>())continue;
        auto& p=v.get("path");auto& s=v.get("subsong");
        if(!p.is<std::string>() || !s.is<double>())continue;
        double n=s.get<double>();if(!std::isfinite(n)||n<0||n>4294967295.0||std::floor(n)!=n)continue;
        Item i{p.get<std::string>(),unsigned(n)};if(valid(i))out.push_back(i);
    }return out;
}
// Insertion boundary is expressed in the original list; preserve selection order.
inline std::vector<size_t> move(std::vector<Item>& items,std::vector<size_t> selection,size_t before) {
    std::sort(selection.begin(),selection.end());selection.erase(std::unique(selection.begin(),selection.end()),selection.end());
    selection.erase(std::remove_if(selection.begin(),selection.end(),[&](size_t n){return n>=items.size();}),selection.end());
    before=(std::min)(before,items.size());std::vector<Item> keep,moved;
    size_t target=before;
    for(size_t n=0;n<items.size();++n) {
        if(std::binary_search(selection.begin(),selection.end(),n)){moved.push_back(items[n]);if(n<before)--target;}
        else keep.push_back(items[n]);
    }
    keep.insert(keep.begin()+target,moved.begin(),moved.end());items.swap(keep);
    std::vector<size_t> result;for(size_t n=0;n<moved.size();++n)result.push_back(target+n);return result;
}
inline void erase(std::vector<Item>& items,const std::vector<size_t>& selection) {
    for(size_t n=items.size();n-->0;)if(std::find(selection.begin(),selection.end(),n)!=selection.end())items.erase(items.begin()+n);
}
inline bool transfer(std::vector<Item>& source,int& sourceCurrent,std::vector<Item>& target,int& targetCurrent,std::vector<size_t> selection,size_t before,bool moving,std::vector<size_t>& inserted){
    inserted.clear();if(&source==&target)return false;
    std::sort(selection.begin(),selection.end());selection.erase(std::unique(selection.begin(),selection.end()),selection.end());
    selection.erase(std::remove_if(selection.begin(),selection.end(),[&](size_t n){return n>=source.size();}),selection.end());if(selection.empty())return false;
    if(target.size()+selection.size()>10000)return false;
    auto from=source,to=target;std::vector<Item> items;for(auto n:selection)items.push_back(source[n]);before=(std::min)(before,to.size());to.insert(to.begin()+before,items.begin(),items.end());
    if(moving)erase(from,selection);
    if(encode(from).serialize().size()+encode(to).serialize().size()>3*1024*1024)return false;
    inserted.reserve(items.size());for(size_t n=0;n<items.size();++n)inserted.push_back(before+n);
    if(moving&&sourceCurrent>=0){if(std::binary_search(selection.begin(),selection.end(),(size_t)sourceCurrent))sourceCurrent=-1;else sourceCurrent-=(int)std::count_if(selection.begin(),selection.end(),[&](size_t n){return n<(size_t)sourceCurrent;});}
    if(targetCurrent>=(int)before)targetCurrent+=(int)items.size();
    source.swap(from);target.swap(to);return true;
}
}
