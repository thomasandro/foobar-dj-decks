#include "../sdk/foobar2000/foo_dj/master_limiter.h"
#include <cassert>
#include <vector>
#include <limits>
#include <iostream>
int main(){
 using dj_master::Limiter;constexpr double pi=3.141592653589793;
 for(unsigned rate:{32000u,44100u,48000u,96000u,192000u})for(double db:{-.3,-1.,-3.,-6.}){
  double ceiling=std::pow(10.,db/20.);Limiter limiter;std::vector<double> audio(rate*2);
  for(unsigned n=0;n<rate;++n){audio[2*n]=2.4*std::sin(2*pi*997*n/rate);audio[2*n+1]=audio[2*n]*.25;}
  auto meter=limiter.process(audio.data(),rate,rate,true,db);assert(meter.input>2.3&&meter.reduction>6);
  for(unsigned n=0;n<rate;++n){assert(std::abs(audio[2*n])<=ceiling+1e-12);assert(std::abs(audio[2*n+1]-audio[2*n]*.25)<1e-12);}
 }
 // Below-threshold unity; bypass preserves finite over-range audio exactly.
 Limiter limiter;double quiet[]={.1,-.2,0,0};auto meter=limiter.process(quiet,2,48000,true);assert(quiet[0]==.1&&quiet[1]==-.2&&meter.reduction==0);
 double loud[]={2.,-4.};meter=limiter.process(loud,1,48000,false);assert(loud[0]==2&&loud[1]==-4&&meter.input==4&&meter.output==4);
 // Single-sample impulse protected immediately, either channel drives the common gain.
 double impulse[]={.5,4};meter=limiter.process(impulse,1,48000,true);assert(meter.reduction>12&&std::abs(impulse[0]*8-impulse[1])<1e-12);
 // Release recovers monotonically and time constants remain sample-rate independent.
 for(unsigned rate:{44100u,48000u,96000u}){Limiter l;double impulse[]={10,10};l.process(impulse,1,rate,true);double last=0;for(unsigned n=0;n<rate;++n){double v[]={.1,.1};l.process(v,1,rate,true);assert(v[0]+1e-14>=last);last=v[0];}assert(last>.09999);}
 // Block boundaries cannot alter the result (including one-frame loop splits).
 std::vector<double> source(10000);for(size_t n=0;n<source.size();++n)source[n]=2.5*std::sin(n*.17);
 auto all=source,split=source;Limiter a,b;a.process(all.data(),all.size()/2,48000,true);
 for(size_t n=0;n<split.size()/2;){size_t count=(std::min)(size_t(1+n%61),split.size()/2-n);b.process(split.data()+2*n,count,48000,true);n+=count;}
 assert(all==split);
 double invalid[]={std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()};meter=limiter.process(invalid,1,48000,true);assert(meter.invalid&&invalid[0]==0&&invalid[1]==0);
 float enormous[]={std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()};meter=limiter.process(enormous,1,0,true);assert(std::isfinite(enormous[0])&&std::abs(enormous[0])<1);
 limiter.reset();double reset[]={.1,.1};limiter.process(reset,1,48000,true);assert(reset[0]==.1);
 std::cout<<"Master limiter: 20 rate/ceiling cases, linked stereo, immediate impulse protection, bypass, unity, release, block invariance, reset and invalid/extreme input passed\n";
}
