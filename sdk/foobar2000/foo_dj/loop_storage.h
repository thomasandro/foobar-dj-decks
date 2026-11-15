#pragma once
#include "ab_loop.h"
#include "picojson.h"
#include <string>
namespace dj_loop {
inline Region restore(const picojson::object& library,const std::string& key,double start,double end){
    Region result;auto item=library.find(key);if(item==library.end()||!item->second.is<picojson::object>())return result;
    const auto& data=item->second.get<picojson::object>();auto a=data.find("a"),b=data.find("b");
    if(a==data.end()||!a->second.is<double>())return result;
    double first=a->second.get<double>();
    if(!std::isfinite(first)||first<0||first<start||(end>0&&first>=end))return result;
    result.a=first;
    if(b!=data.end()&&b->second.is<double>()){
        double last=b->second.get<double>();
        if(std::isfinite(last)&&last-first>=0.1&&(end<=0||last<=end))result.b=last;
    }
    return result; // Never restore enabled state or change transport.
}
inline void remember(picojson::object& library,const std::string& key,const Region& region){
    if(!std::isfinite(region.a)||region.a<0){library.erase(key);return;}
    picojson::object points;points["a"]=picojson::value(region.a);
    if(region.valid())points["b"]=picojson::value(region.b);
    library[key]=picojson::value(points);
}
}
