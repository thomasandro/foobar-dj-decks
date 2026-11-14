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
}
