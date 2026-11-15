#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/preview_visual.h"
int main(){
    dj_preview::Visual visual;visual.reset(4);visual.begin(4,{{0,8},{46,8}},100);
    double samples[]={.5,-.2,.1,-.8};visual.add(4,0,1000,samples,2,2);
    auto state=visual.snapshot(0);assert(state.duration==16&&state.trackDuration==100&&state.known[0]&&state.peaks[0]==.8f);
    visual.submitted(4,10);state=visual.snapshot(.5);assert(state.position==9.5&&state.sourcePosition==47.5);
    state=visual.snapshot(20);assert(state.position==0&&state.sourcePosition==0);
    visual.submitted(4,100);state=visual.snapshot(0);assert(state.position==16&&state.sourcePosition==54);
    visual.reset(5);visual.begin(5,{{10,15}});visual.add(4,0,1000,samples,2,2);visual.submitted(4,10);
    state=visual.snapshot(0);assert(state.position==0&&!state.known[0]&&state.sourcePosition==10);
    double invalid[]={NAN,INFINITY};visual.add(5,0,1000,invalid,1,2);state=visual.snapshot(0);assert(state.known[0]&&state.peaks[0]==0);
    std::array<float,32> peaks{};std::array<bool,32> known{};peaks[31]=.7f;known[31]=true;
    visual.reset(6);visual.overview(6,100,peaks,known,false);visual.begin(6,{{46,8}},100);
    state=visual.snapshot(0);assert(state.sourcePosition==46&&state.trackDuration==100&&state.known[1023]); // overview survives late preview metadata
    visual.overview(5,3,peaks,known,true);assert(visual.snapshot(0).trackDuration==100);
    visual.restart(7,70);state=visual.snapshot(0);assert(state.sourcePosition==70&&state.known[1023]&&state.trackDuration==100);
    visual.submitted(6,14);assert(visual.snapshot(0).submitted==0);
    visual.begin(7,{{70,8}},100);visual.submitted(7,2);state=visual.snapshot(0);assert(state.sourcePosition==72&&state.known[1023]);
    std::cout<<"Progressive preview visual: bounded peaks, stereo/invalid samples, combined source offsets, latency adjustment, end clamping and stale-request rejection passed\n";
}
