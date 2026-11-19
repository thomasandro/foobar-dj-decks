#pragma once
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
namespace dj_midi {
enum Action {crossfade,volumeA,volumeB,tempoA,tempoB,playA,playB,both,startA,startB,unloadA,unloadB,cueA,cueB,loopAA,loopAB,loopA,loopBA,loopBB,loopB,bpmA,bpmB,keyA,keyB,jogA,jogB,count};
inline bool jog(int a){return a==jogA||a==jogB;}
inline bool continuous(int action){return (action>=0&&action<=tempoB)||jog(action);}
enum Mode {absolute,twosComplement,binaryOffset,signedMagnitude,offset16};
inline int delta(int value,int mode){if(value<0||value>127)return 0;switch(mode){case twosComplement:return value<64?value:value-128;case binaryOffset:return value==0?0:value-64;case signedMagnitude:return value<64?value:64-value;case offset16:return value==0||value>31?0:value-16;default:return 0;}}
inline double step(int action,int sensitivity){static const double knobs[]={.001,.0025,.005,.01,.02,.05},seconds[]={.01,.025,.05,.1,.25,1};return (jog(action)?seconds:knobs)[std::clamp(sensitivity,0,5)];}
inline const wchar_t* name(int action,bool en){
 static const wchar_t* english[]={L"Crossfader",L"A: volume",L"B: volume",L"A: tempo",L"B: tempo",L"A: play / pause",L"B: play / pause",L"A+B: toggle playback",L"A: return to start",L"B: return to start",L"A: unload",L"B: unload",L"A: headphone Cue",L"B: headphone Cue",L"A: set loop A",L"A: set loop B",L"A: loop on / off",L"B: set loop A",L"B: set loop B",L"B: loop on / off",L"A: match BPM",L"B: match BPM",L"A: match key",L"B: match key",L"A: jog",L"B: jog"};
 static const wchar_t* french[]={L"Crossfader",L"A : volume",L"B : volume",L"A : vitesse",L"B : vitesse",L"A : lecture / pause",L"B : lecture / pause",L"A+B : inverser les lectures",L"A : revenir au dÃ©but",L"B : revenir au dÃ©but",L"A : dÃ©charger",L"B : dÃ©charger",L"A : prÃ©Ã©coute Cue",L"B : prÃ©Ã©coute Cue",L"A : poser le point A",L"A : poser le point B",L"A : boucle oui / non",L"B : poser le point A",L"B : poser le point B",L"B : boucle oui / non",L"A : caler le BPM",L"B : caler le BPM",L"A : caler la tonalitÃ©",L"B : caler la tonalitÃ©",L"A : jog (dÃ©placement)",L"B : jog (dÃ©placement)"};
 return action>=0&&action<count?(en?english[action]:french[action]):L"?";
}
struct Event {int kind=0,channel=0,number=0,value=0,maximum=127;bool valid=false;};
inline Event decode(unsigned packed){
 unsigned status=packed&255,a=(packed>>8)&255,b=(packed>>16)&255;Event e;e.channel=status&15;
 if(status<128||a>127||b>127)return e;
 switch(status&240){case 0x80:case 0x90:e.kind=9;e.number=a;e.value=(status&240)==0x80?0:b;break;case 0xb0:e.kind=11;e.number=a;e.value=b;break;case 0xe0:e.kind=14;e.number=0;e.value=a+(b<<7);e.maximum=16383;break;default:return e;}
 e.valid=true;return e;
}
struct Binding {int kind=0,channel=0,number=0;bool inverted=false;int mode=absolute,sensitivity=2;bool matches(const Event& e)const{return e.valid&&kind==e.kind&&channel==e.channel&&number==e.number;}};
inline bool valid(const Binding& b){return b.mode>=absolute&&b.mode<=offset16&&b.sensitivity>=0&&b.sensitivity<=5&&(b.mode==absolute||b.kind==11)&&(b.kind==9||b.kind==11||b.kind==14)&&b.channel>=0&&b.channel<16&&b.number>=0&&b.number<128&&(b.kind!=14||b.number==0);}
inline bool validFor(int a,const Binding& b){return valid(b)&&(!continuous(a)||(b.kind!=9))&&(!jog(a)||b.mode!=absolute)&&(continuous(a)||b.mode==absolute);}
struct Profile {std::string device;std::array<Binding,count> bindings{};};
// Small versioned, bounded text format; device identifiers are quoted (Unicode UTF-8).
inline std::string save(const std::vector<Profile>& profiles){std::ostringstream out;out<<"DJMIDI 2\n";for(const auto& p:profiles)for(int a=0;a<count;++a){auto b=p.bindings[a];if(valid(b))out<<std::quoted(p.device)<<' '<<a<<' '<<b.kind<<' '<<b.channel<<' '<<b.number<<' '<<int(b.inverted)<<' '<<b.mode<<' '<<b.sensitivity<<'\n';}return out.str();}
inline bool load(const std::string& text,std::vector<Profile>& result){
 if(text.empty()){result.clear();return true;}if(text.size()>65536)return false;
 std::istringstream in(text);std::string magic;int version;if(!(in>>magic>>version)||magic!="DJMIDI"||(version!=1&&version!=2))return false;
 std::vector<Profile> next;while(in>>std::ws&&!in.eof()){
  std::string device;int action,kind,channel,number,inverted;
  if(!(in>>std::quoted(device)>>action>>kind>>channel>>number>>inverted)||device.empty()||device.size()>1024||action<0||action>=count||(inverted!=0&&inverted!=1))return false;
  Binding b{kind,channel,number,inverted!=0};if(version==2&&!(in>>b.mode>>b.sensitivity))return false;if(!validFor(action,b))return false;
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
 std::array<Binding,count> bindings{};int learning=-1,learnMode=absolute,learnSensitivity=2;bool pickup=true;
 Router(){reset();}
 void reset(){pressed.fill(false);latched.fill(false);previous.fill(-1);}
 void arm(int a,int mode=absolute,int sensitivity=2){learning=a>=0&&a<count?a:-1;learnMode=mode;learnSensitivity=sensitivity;reset();}
 void forget(int a){if(a>=0&&a<count)bindings[a]={};learning=-1;reset();}
 template<class Current,class Apply> bool receive(unsigned packed,Current current,Apply apply){
  auto e=decode(packed);if(!e.valid)return false;
  if(learning>=0){int a=learning;Binding candidate{e.kind,e.channel,e.number,false,learnMode,learnSensitivity};
   if(!validFor(a,candidate)||(learnMode!=absolute&&delta(e.value,learnMode)==0)||(!continuous(a)&&e.value<(e.kind==9?1:(e.maximum+1)/2)))return false;
   for(auto& b:bindings)if(b.matches(e))b={};bindings[a]=candidate;learning=-1;reset();
   // Suppress this learning gesture until its release; do not trigger transport.
   if(!continuous(a))pressed[a]=true;return true;
  }
  for(int a=0;a<count;++a)if(bindings[a].matches(e)){
   const auto& mapping=bindings[a];
   if(mapping.mode!=absolute){int ticks=std::clamp(delta(e.value,mapping.mode),-16,16);if(mapping.inverted)ticks=-ticks;if(!ticks)continue;
    double amount=ticks*step(a,mapping.sensitivity);
    if(jog(a))apply(a,amount);else{double value=current(a);if(std::isfinite(value))apply(a,std::clamp(value+amount,0.,1.));}continue;
   }
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
inline std::wstring modeName(int mode,bool en){switch(mode){case twosComplement:return L"Rel. 1 / 127 (2's complement)";case binaryOffset:return L"Rel. 65 / 63 (offset 64)";case signedMagnitude:return L"Rel. 1 / 65 (signed magnitude)";case offset16:return L"Rel. 17 / 15 (offset 16)";default:return en?L"Absolute":L"Absolu";}}
inline std::wstring describe(const Binding& b,bool en){if(!valid(b))return en?L"Not assigned":L"Non associÃ©";return std::wstring(b.kind==9?L"Note ":b.kind==11?L"CC ":L"Pitch Bend ")+(b.kind==14?L"":std::to_wstring(b.number)+L" Â· ")+(en?L"channel ":L"canal ")+std::to_wstring(b.channel+1)+(b.mode==absolute?L"":L" Â· "+modeName(b.mode,en));}
}
