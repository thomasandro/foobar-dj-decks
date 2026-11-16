#pragma once
namespace dj_bpm {
// 0 auto prefers Smart Tempo; explicit selections never silently switch.
inline int select(int preference,bool smart,bool classic){
 if(preference==1)return smart?1:-1;if(preference==2)return classic?2:-1;
 return smart?1:(classic?2:-1);
}
}
