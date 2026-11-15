#pragma once
#include "queue_format.h"
#include <cstring>
#include <cctype>
class DjQueueDurationHook:public titleformat_hook {
    double seconds;
public:
    explicit DjQueueDurationHook(double value):seconds(value){}
    bool process_field(titleformat_text_out* out,const char* name,t_size length,bool& found)override{
        if(!name)return false;
        // SIZE_MAX is the SDK convention for a null-terminated name. Passing it
        // to std::string(ptr, count) throws and used to trigger QueueLabel's fallback.
        if(length==SIZE_MAX)length=std::strlen(name);
        if(length!=6&&length!=14)return false;
        std::string field(name,length);std::transform(field.begin(),field.end(),field.begin(),[](unsigned char c){return (char)std::tolower(c);});
        if(field!="length"&&field!="length_seconds")return false;
        found=std::isfinite(seconds)&&seconds>0&&seconds<=1e12;
        std::string text=dj_queue_format::duration(seconds);
        if(field=="length_seconds"&&found){char b[80];std::snprintf(b,sizeof(b),"%.3f",seconds);text=b;}
        out->write(titleformat_inputtypes::meta,text.c_str());return true;
    }
    bool process_function(titleformat_text_out*,const char*,t_size,titleformat_hook_function_params*,bool&)override{return false;}
};
