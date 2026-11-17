#pragma once
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
namespace dj_midi {
enum Action {crossfade,volumeA,volumeB,tempoA,tempoB,playA,playB,both,startA,startB,unloadA,unloadB,cueA,cueB,loopAA,loopAB,loopA,loopBA,loopBB,loopB,bpmA,bpmB,keyA,keyB,count};
inline bool continuous(int action){return action>=0&&action<=tempoB;}
inline const wchar_t* name(int action,bool en){
 static const wchar_t* english[]={L"Crossfader",L"A: volume",L"B: volume",L"A: tempo",L"B: tempo",L"A: play / pause",L"B: play / pause",L"A+B: toggle playback",L"A: return to start",L"B: return to start",L"A: unload",L"B: unload",L"A: headphone Cue",L"B: headphone Cue",L"A: set loop A",L"A: set loop B",L"A: loop on / off",L"B: set loop A",L"B: set loop B",L"B: loop on / off",L"A: match BPM",L"B: match BPM",L"A: match key",L"B: match key"};
 static const wchar_t* french[]={L"Crossfader",L"A : volume",L"B : volume",L"A : vitesse",L"B : vitesse",L"A : lecture / pause",L"B : lecture / pause",L"A+B : inverser les lectures",L"A : revenir au début",L"B : revenir au début",L"A : décharger",L"B : décharger",L"A : préécoute Cue",L"B : préécoute Cue",L"A : poser le point A",L"A : poser le point B",L"A : boucle oui / non",L"B : poser le point A",L"B : poser le point B",L"B : boucle oui / non",L"A : caler le BPM",L"B : caler le BPM",L"A : caler la tonalité",L"B : caler la tonalité"};
 return action>=0&&action<count?(en?english[action]:french[action]):L"?";
}
struct Event {int kind=0,channel=0,number=0,value=0,maximum=127;bool valid=false;};
inline Event decode(unsigned packed){
 unsigned status=packed&255,a=(packed>>8)&255,b=(packed>>16)&255;Event e;e.channel=status&15;
 if(status<128||a>127||b>127)return e;
 switch(status&240){case 0x80:case 0x90:e.kind=9;e.number=a;e.value=(status&240)==0x80?0:b;break;case 0xb0:e.kind=11;e.number=a;e.value=b;break;case 0xe0:e.kind=14;e.number=0;e.value=a+(b<<7);e.maximum=16383;break;default:return e;}
 e.valid=true;return e;
}
struct Binding {int kind=0,channel=0,number=0;bool inverted=false;bool matches(const Event& e)const{return e.valid&&kind==e.kind&&channel==e.channel&&number==e.number;}};
inline bool valid(const Binding& b){return (b.kind==9||b.kind==11||b.kind==14)&&b.channel>=0&&b.channel<16&&b.number>=0&&b.number<128&&(b.kind!=14||b.number==0);}
struct Profile {std::string device;std::array<Binding,count> bindings{};};
// Small versioned, bounded text format; device identifiers are quoted (Unicode UTF-8).
inline std::string save(const std::vector<Profile>& profiles){std::ostringstream out;out<<"DJMIDI 1\n";for(const auto& p:profiles)for(int a=0;a<count;++a){auto b=p.bindings[a];if(valid(b))out<<std::quoted(p.device)<<' '<<a<<' '<<b.kind<<' '<<b.channel<<' '<<b.number<<' '<<int(b.inverted)<<'\n';}return out.str();}
inline bool load(const std::string& text,std::vector<Profile>& result){
 if(text.empty()){result.clear();return true;}if(text.size()>65536)return false;
 std::istringstream in(text);std::string magic;int version;if(!(in>>magic>>version)||magic!="DJMIDI"||version!=1)return false;
 std::vector<Profile> next;while(in>>std::ws&&!in.eof()){
  std::string device;int action,kind,channel,number,inverted;
  if(!(in>>std::quoted(device)>>action>>kind>>channel>>number>>inverted)||device.empty()||device.size()>1024||action<0||action>=count||(inverted!=0&&inverted!=1))return false;
  Binding b{kind,channel,number,inverted!=0};if(!valid(b)||(continuous(action)&&kind==9))return false;
  auto p=std::find_if(next.begin(),next.end(),[&](const Profile& p){return p.device==device;});
  if(p==next.end()){if(next.size()>=16)return false;next.push_back({device});p=next.end()-1;}
  if(valid(p->bindings[action]))return false;
  for(auto& old:p->bindings)if(old.kind==kind&&old.channel==channel&&old.number==number)return false;
  p->bindings[action]=b;
 }result=std::move(next);return true;
}
class Router {
 std::array<bool,count> pressed{},latched{};std::array<double,count> previous{};
public:
 std::array<Binding,count> bindings{};int learning=-1;bool pickup=true;
 Router(){reset();}
 void reset(){pressed.fill(false);latched.fill(false);previous.fill(-1);}
 void arm(int a){learning=a>=0&&a<count?a:-1;reset();}
 void forget(int a){if(a>=0&&a<count)bindings[a]={};learning=-1;reset();}
 template<class Current,class Apply> bool receive(unsigned packed,Current current,Apply apply){
  auto e=decode(packed);if(!e.valid)return false;
  if(learning>=0){int a=learning;if((continuous(a)&&e.kind==9)||(!continuous(a)&&e.value<(e.kind==9?1:(e.maximum+1)/2)))return false;
   for(auto& b:bindings)if(b.matches(e))b={};bindings[a]={e.kind,e.channel,e.number,false};learning=-1;reset();
   // Suppress this learning gesture until its release; do not trigger transport.
   if(!continuous(a))pressed[a]=true;return true;
  }
  for(int a=0;a<count;++a)if(bindings[a].matches(e)){
   double value=double(e.value)/e.maximum;if(bindings[a].inverted)value=1-value;
   if(continuous(a)){
    double target=std::clamp(current(a),0.,1.);double tolerance=1.5/e.maximum;
    if(latched[a]&&previous[a]>=0&&std::abs(previous[a]-target)>tolerance*2)latched[a]=false;
    bool crossed=previous[a]>=0&&(previous[a]-target)*(value-target)<=0;
    if(!pickup||latched[a]||crossed||std::abs(value-target)<=tolerance){latched[a]=true;apply(a,value);}
    previous[a]=value;
   }else{bool down=e.kind==9?e.value>0:value>=.5;if(down&&!pressed[a])apply(a,1.);pressed[a]=down;}
  }return false;
 }
};
inline std::wstring describe(const Binding& b,bool en){if(!valid(b))return en?L"Not assigned":L"Non associé";return std::wstring(b.kind==9?L"Note ":b.kind==11?L"CC ":L"Pitch Bend ")+(b.kind==14?L"":std::to_wstring(b.number)+L" · ")+(en?L"channel ":L"canal ")+std::to_wstring(b.channel+1);}
}
