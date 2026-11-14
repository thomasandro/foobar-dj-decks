#include "../sdk/foobar2000/foo_dj/beat_sync.h"
#include "../sdk/foobar2000/foo_dj/tempo_match.h"
#include <cassert>
#include <iostream>
#include <random>
#include <limits>
std::vector<float> drums(double bpm,double offset){
 std::vector<float> rms;double period=60/bpm;
 for(int bucket=0;bucket<4000;bucket++){double sum=0;for(int n=0;n<480;n++){double t=(bucket*480+n)/48000.0, age=t-offset;double v=0;if(age>=0){age=std::fmod(age,period);v=std::exp(-age/0.035)*std::sin(2*3.141592653589793*75*age);double hat=std::fmod(t-offset+period/2,period);v+=0.16*std::exp(-hat/0.012)*std::sin(2*3.141592653589793*2300*hat);}sum+=v*v;}rms.push_back((float)std::sqrt(sum/480));}return rms;
}
double circular(double a,double b){double d=std::abs(dj_beats::fraction(a)-dj_beats::fraction(b));return(std::min)(d,1-d);}
int main(){
 auto a=drums(128,0.073),b=drums(120,0.211);auto ga=dj_beats::fit(a,128,18),gb=dj_beats::fit(b,120,20);
 std::cout<<"Grid A offset="<<ga.offset<<" confidence="<<ga.confidence<<"; B offset="<<gb.offset<<" confidence="<<gb.confidence<<"\n";
 assert(ga.valid&&gb.valid);assert(circular(ga.offset/ga.period,0.073/ga.period)*ga.period<0.025);assert(circular(gb.offset/gb.period,0.211/gb.period)*gb.period<0.025);
 double target;assert(dj_beats::align(ga,gb,18.37,20.12,40,target));
 assert(circular((target-0.073)/ga.period,(20.12-0.211)/gb.period)*ga.period<0.025);
 assert(std::abs(target-18.37)<=ga.period/2+0.01);
 double speed=1;assert(dj_tempo::match(128,120,1.08,speed)==dj_tempo::Match::ok);assert(std::abs(speed*128-120*1.08)<1e-9);
 assert(!dj_beats::fit(std::vector<float>(4000,0),120,20).valid);assert(!dj_beats::fit(std::vector<float>(4000,0.5),120,20).valid);assert(!dj_beats::fit(std::vector<float>(100,0.5),120,0).valid);
 std::mt19937 rng(12);std::uniform_real_distribution<float> noise(0,0.4);std::vector<float> irregular(4000);for(auto&v:irregular)v=noise(rng);assert(!dj_beats::fit(irregular,120,20).valid);
 assert(!dj_beats::fit(a,0,20).valid);assert(!dj_beats::align({},gb,1,1,40,target));
 assert(dj_beats::align(ga,gb,0.01,20.12,40,target)&&target>=0);assert(dj_beats::align(ga,gb,39.89,20.12,40,target)&&target<=39.9);
 auto manual=dj_beats::manual(120,12.237);assert(manual.valid&&std::abs(manual.offset-0.237)<1e-9);
 auto backward=dj_beats::manual(120,manual.offset-0.01);auto forward=dj_beats::manual(120,backward.offset+0.01);assert(std::abs(forward.offset-manual.offset)<1e-9);
 auto wrap=dj_beats::manual(120,-0.01);assert(wrap.valid&&std::abs(wrap.offset-0.49)<1e-9);
 assert(!dj_beats::manual(0,1).valid);assert(!dj_beats::manual(120,std::numeric_limits<double>::quiet_NaN()).valid);
 auto userA=dj_beats::manual(128,0.073),userB=dj_beats::manual(120,0.211);
 assert(dj_beats::align(userA,userB,18.37,20.12,40,target));assert(circular((target-0.073)/userA.period,(20.12-0.211)/userB.period)<1e-10);
 assert(!dj_beats::Grid{}.valid);
 for(auto pair:{std::pair<double,double>{86,172},{172,86},{100,180}}){
   double ownSpeed=1,refSpeed=pair.second==180?1.1:1,factor=1;
   assert(dj_tempo::match(pair.first,pair.second,refSpeed,ownSpeed,16,&factor)==dj_tempo::Match::ok);
   auto own=dj_beats::manual(pair.first,.073),reference=dj_beats::manual(pair.second,.211);reference.period/=factor;
   assert(dj_beats::align(own,reference,18.37,20.12,200,target));
   for(double elapsed:{0.,1.,7.1,30.})assert(circular((target+ownSpeed*elapsed-own.offset)/own.period,(20.12+refSpeed*elapsed-reference.offset)/reference.period)<1e-9);
 }
 std::cout<<"Manual grids passed: anchor, millisecond shifts, wrap, invalid values and SYNC\n";
 auto detail=dj_beats::ticks(gb,0.0,4.0,400);assert(detail.stride==1&&detail.pixels.size()==8);
 for(size_t i=1;i<detail.pixels.size();i++)assert(detail.pixels[i]-detail.pixels[i-1]>=49);
 auto overview=dj_beats::ticks(gb,0.0,300.0,240);assert(overview.stride>1&&overview.pixels.size()<=32);
 for(int x:overview.pixels)assert(x>=0&&x<240);
 auto moved=dj_beats::ticks(gb,5.0,4.0,400);assert(moved.pixels.size()==8);
 assert(dj_beats::ticks({},0,4,400).pixels.empty());assert(dj_beats::ticks(gb,0,0,400).pixels.empty());assert(dj_beats::ticks(gb,0,4,0).pixels.empty());
 std::cout<<"Beat-marker tests passed: zoom, overview, scrolling, invalid grids\n";
 std::cout<<"Synthetic drums: detected beat phase and alignment within 25 ms; silence, flat audio, irregular signal and boundaries passed\n";
}
