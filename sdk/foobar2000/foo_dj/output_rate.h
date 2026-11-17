#pragma once
namespace dj_output {
inline unsigned rate(unsigned forced,unsigned preference,unsigned source){
 if(forced>=8000&&forced<=768000)return forced;
 if(preference==44100||preference==48000||preference==96000||preference==192000)return preference;
 return source>=8000&&source<=768000?source:44100;
}
}
