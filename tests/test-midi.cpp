#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <cassert>
#include <iostream>
#include <thread>
#include "../sdk/foobar2000/foo_dj/midi_control.h"
// A fake WinMM boundary: no installed MIDI device is enumerated or opened.
namespace fake {
using Callback=void(CALLBACK*)(HMIDIIN,UINT,DWORD_PTR,DWORD_PTR,DWORD_PTR); static Callback callback=nullptr;static DWORD_PTR instance=0;static int fail=0,opens=0,starts=0,stops=0,closes=0;
UINT WINAPI devices(){return 1;}
MMRESULT WINAPI caps(UINT_PTR,LPMIDIINCAPSW out,UINT){*out={};out->wMid=3;out->wPid=7;wcscpy_s(out->szPname,L"Fake controller");return 0;}
MMRESULT WINAPI open(LPHMIDIIN h,UINT,DWORD_PTR cb,DWORD_PTR context,DWORD){++opens;if(fail==1)return MMSYSERR_ALLOCATED;*h=(HMIDIIN)1;callback=(Callback)cb;instance=context;return 0;}
MMRESULT WINAPI start(HMIDIIN){++starts;return fail==2?MMSYSERR_ERROR:0;}
MMRESULT WINAPI stop(HMIDIIN){++stops;return 0;}MMRESULT WINAPI reset(HMIDIIN){return 0;}MMRESULT WINAPI close(HMIDIIN){++closes;return 0;}
void emit(unsigned packed){callback((HMIDIIN)1,MIM_DATA,instance,packed,0);}
}
#define midiInGetNumDevs fake::devices
#define midiInGetDevCapsW fake::caps
#define midiInOpen fake::open
#define midiInStart fake::start
#define midiInStop fake::stop
#define midiInReset fake::reset
#define midiInClose fake::close
#include "../sdk/foobar2000/foo_dj/midi_input.h"
#undef midiInGetNumDevs
#undef midiInGetDevCapsW
#undef midiInOpen
#undef midiInStart
#undef midiInStop
#undef midiInReset
#undef midiInClose
unsigned message(unsigned status,unsigned number,unsigned value){return status|(number<<8)|(value<<16);}
int main(){using namespace dj_midi;
 assert(!decode(0xf8).valid&&!decode(message(0xa0,1,2)).valid&&!decode(message(0xb0,128,2)).valid);
 assert(decode(message(0x91,60,127)).channel==1&&decode(message(0x81,60,100)).value==0);
 assert(decode(message(0xe0,127,127)).value==16383);
 Router router;int called=0,last=-1;double values[count]={};for(auto& v:values)v=.5;
 auto current=[&](int a){return values[a];};auto apply=[&](int a,double v){++called;last=a;values[a]=v;};
 router.arm(playA);assert(router.receive(message(0x91,60,127),current,apply)&&called==0);
 router.receive(message(0x91,60,100),current,apply);assert(!called);router.receive(message(0x81,60,10),current,apply);
 router.receive(message(0x91,60,127),current,apply);router.receive(message(0x91,60,127),current,apply);assert(called==1&&last==playA);
 router.receive(message(0x91,60,0),current,apply);router.receive(message(0x90,60,127),current,apply);assert(called==1);
 router.receive(message(0x91,60,1),current,apply);assert(called==2);
 router.arm(volumeA);assert(!router.receive(message(0x90,50,127),current,apply)&&router.learning==volumeA);
 assert(router.receive(message(0xb0,7,0),current,apply));assert(called==2);
 router.receive(message(0xb0,7,10),current,apply);assert(called==2);
 router.receive(message(0xb0,7,90),current,apply);assert(called==3&&last==volumeA);
 router.receive(message(0xb0,7,100),current,apply);assert(called==4);
 values[volumeA]=.2;router.receive(message(0xb0,7,110),current,apply);assert(called==4);
 router.receive(message(0xb0,7,0),current,apply);assert(called==5);
 router.pickup=false;router.bindings[volumeA].inverted=true;router.receive(message(0xb0,7,0),current,apply);assert(values[volumeA]==1);
 router.arm(playB);router.receive(message(0xb0,8,0),current,apply);assert(router.learning==playB);router.receive(message(0xb0,8,127),current,apply);int before=called;
 router.receive(message(0xb0,8,127),current,apply);router.receive(message(0xb0,8,0),current,apply);router.receive(message(0xb0,8,127),current,apply);assert(called==before+1&&last==playB);
 router.arm(tempoA);router.receive(message(0xe2,0,64),current,apply);router.receive(message(0xe2,127,127),current,apply);assert(values[tempoA]==1);
 router.arm(tempoB);router.receive(message(0xe2,0,64),current,apply);assert(!valid(router.bindings[tempoA])&&valid(router.bindings[tempoB]));
 router.forget(tempoB);assert(!valid(router.bindings[tempoB])&&router.learning==-1);
 std::vector<Profile> profiles={{"Contrôleur 日本語",router.bindings}};auto saved=save(profiles);std::vector<Profile> loaded;assert(load(saved,loaded)&&save(loaded)==saved);
 for(auto bad:{"garbage","DJMIDI 2","DJMIDI 1\n\"x\" 99 11 0 7 0","DJMIDI 1\n\"x\" 0 9 0 7 0","DJMIDI 1\n\"x\" 0 11 0 7 2","DJMIDI 1\n\"x\" 0 11 0 7 0\n\"x\" 1 11 0 7 0"}){auto prior=save(loaded);assert(!load(bad,loaded)&&save(loaded)==prior);}
 assert(!load(std::string(65537,'x'),loaded));
 auto ports=devices();assert(ports.size()==1&&ports[0].key=="3:7:Fake controller#0");
 {Input input,second;assert(input.open(0)==0&&input.opened());assert(second.open(0)==MMSYSERR_ALLOCATED);fake::emit(message(0xb0,7,42));bool lost;auto pending=input.drain(lost);assert(!lost&&pending.size()==1);for(int n=0;n<300;++n)fake::emit(message(0xb0,7,n%128));pending=input.drain(lost);assert(lost&&pending.size()==256);input.clear();assert(input.drain(lost).empty()&&!lost);input.close();assert(!input.opened());assert(second.open(0)==0);second.close();fake::fail=1;assert(input.open(0)!=0&&!input.opened());fake::fail=2;assert(input.open(0)!=0&&!input.opened());fake::fail=0;assert(second.open(0)==0);}
 assert(fake::closes==4);
 std::cout<<"MIDI: Note/CC edges, channel isolation, Learn suppression/conflicts, absolute CC/Pitch Bend, soft takeover/manual changes/inversion, Unicode persistence and strict rollback; fake WinMM input, single-panel ownership, bounded overflow, retry and teardown passed. No host or MIDI device accessed.\n";
}

