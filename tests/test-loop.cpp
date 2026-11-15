#include "../sdk/foobar2000/foo_dj/ab_loop.h"
#include "../sdk/foobar2000/foo_dj/beat_sync.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
struct Abort{};
struct Input{double position=0;void seek(double t,Abort&){position=t;}};
struct Stems{double position=0;void seek(double t){position=t;}};
struct Deck{dj_loop::Region loop;std::shared_ptr<Input> helper=std::make_shared<Input>();std::shared_ptr<Stems> stems;
 std::vector<double> src,pending;double srcPos=0,framesPlayed=0;size_t pendingPos=0;bool eof=false,stretchFlushed=false;std::shared_ptr<int> stretch;};
int main(){
 double target=0;assert(dj_loop::snap(.5,.125,1,1.3,0,10,target)&&target==1.125);
 assert(dj_loop::snap(.5,.125,1,1.4,0,10,target)&&target==1.625);
 assert(dj_loop::snap(.5,.125,4,3.4,0,10,target)&&target==4.125);
 assert(dj_loop::snap(.5,.125,1,1.13,1.225,10,target)&&target==1.625);
 assert(!dj_loop::snap(.5,.125,1,1.3,1.2,1.5,target));
 assert(!dj_loop::snap(0,0,1,1,0,10,target));
 bool hint=false;auto g=dj_beats::displayed({},120,false,0,hint);assert(hint&&g.valid&&g.period==.5);
 g=dj_beats::displayed({},120,true,.125,hint);assert(!hint&&g.valid&&g.offset==.125);
 auto visible=dj_beats::ticks(g,0,20,100);assert(dj_loop::snap(g.period,g.offset,visible.stride,4.8,0,20,target));
 assert(std::fmod((target-g.offset)/g.period,visible.stride)==0);
 g=dj_beats::displayed({},0,false,0,hint);assert(!g.valid);

 dj_loop::Region region;assert(!region.valid());region.setA(2);assert(!region.setB(2.05));assert(region.setB(3));region.enabled=true;
 assert(region.cap(2.5,1,48000,1024)==1024&&region.cap(3,1,48000,1024)==0);
 region.setA(4);assert(!region.enabled&&region.b<0);
 Abort abort;
 for(unsigned rate:{44100,48000,96000})for(double speed:{.5,1.,1.5})for(bool stems:{false,true}){
  Deck a,b;a.loop={.1,.213,true};b.loop={.2,.337,true};a.framesPlayed=.1*rate;b.framesPlayed=.2*rate;
  if(stems){a.stems=std::make_shared<Stems>();b.stems=std::make_shared<Stems>();}
  unsigned wraps=0;size_t sent=0;
  for(int block=0;block<300;++block){for(auto d:{&a,&b}){bool edge=!d->loop.contains(d->framesPlayed/rate);d->eof=edge;d->src={1};d->pending={2};d->stretch=std::make_shared<int>(1);
    dj_loop::wrap(*d,rate,abort);if(edge){++wraps;assert(!d->eof&&d->src.empty()&&d->pending.empty()&&!d->stretch);
      assert((stems?d->stems->position:d->helper->position)==d->loop.a);}}
   size_t frames=a.loop.cap(a.framesPlayed/rate,speed,rate,1024);frames=b.loop.cap(b.framesPlayed/rate,1.,rate,frames);assert(frames>0&&frames<=1024);
   a.framesPlayed+=frames*speed;b.framesPlayed+=frames;sent+=frames;
   assert(a.framesPlayed/rate<=a.loop.b+speed/rate+1e-10);assert(b.framesPlayed/rate<=b.loop.b+1./rate+1e-10);
  }assert(wraps>10&&sent>0);
 }
 std::cout<<"A/B loop audio boundary helpers: 18 rate/speed/source combinations, 5400 shared blocks, independent wraps, EOF, original/stems seek and lookahead reset passed (mock readers)\n";
}
