#pragma once
#include "deck_queue.h"
#include "ab_loop.h"
#include <array>
#include <stdexcept>
#include <set>
namespace dj_session {
inline constexpr size_t maxBytes=16*1024*1024;
struct Playlist {std::vector<dj_queue::Item> items;int current=-1;};
struct Deck {dj_queue::Item item;double position=0,volume=1,speed=1;int range=16,transpose=0;bool fixed=true,snap=false;dj_loop::Region loop;bool manual=false;double phase=0;};
struct Marker {dj_queue::Item item;dj_loop::Region loop;bool manual=false;double phase=0;};
struct Bundle {std::array<Playlist,2> lists;bool full=false;std::array<Deck,2> decks;double crossfade=.5,fade=5;bool linear=false;std::vector<Marker> markers;};
inline void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
inline double number(const picojson::value& v,double lo,double hi){require(v.is<double>(),"Expected a number");auto n=v.get<double>();require(std::isfinite(n)&&n>=lo&&n<=hi,"Number outside supported range");return n;}
inline int integer(const picojson::value& v,int lo,int hi){double n=number(v,lo,hi);require(std::floor(n)==n,"Expected an integer");return (int)n;}
inline bool boolean(const picojson::value& v){require(v.is<bool>(),"Expected a boolean");return v.get<bool>();}
inline dj_queue::Item track(const picojson::value& v,bool empty=false){require(v.is<picojson::object>(),"Invalid track record");auto& p=v.get("path");require(p.is<std::string>(),"Missing track path");auto n=number(v.get("subsong"),0,4294967295.);require(n==std::floor(n),"Invalid subsong");dj_queue::Item out{p.get<std::string>(),(unsigned)n};require((empty&&out.path.empty())||dj_queue::valid(out),"Invalid track path");return out;}
inline picojson::value trackValue(const dj_queue::Item& item){picojson::object o;o["path"]=picojson::value(item.path);o["subsong"]=picojson::value(double(item.subsong));return picojson::value(o);}
inline picojson::value loopValue(const dj_loop::Region& loop){picojson::object o;if(loop.a>=0){o["a"]=picojson::value(loop.a);if(loop.b>=0)o["b"]=picojson::value(loop.b);}return picojson::value(o);}
inline dj_loop::Region loopValue(const picojson::value& v){require(v.is<picojson::object>(),"Invalid loop record");dj_loop::Region out;const auto& a=v.get("a");const auto& b=v.get("b");if(a.is<picojson::null>()){require(b.is<picojson::null>(),"Loop B without A");return out;}out.a=number(a,0,1e12);if(!b.is<picojson::null>()){out.b=number(b,0,1e12);require(out.valid(),"Invalid loop interval");}return out;}
inline dj_loop::Region bounded(dj_loop::Region points,double start,double end){points.enabled=false;if(points.a<start||(end>0&&points.a>=end)){points={};return points;}if(points.b>=0&&(!points.valid()||(end>0&&points.b>end)))points.b=-1;return points;}
inline int currentFor(const Playlist& list,const dj_queue::Item& item){auto same=[&](int i){return i>=0&&i<(int)list.items.size()&&list.items[i].path==item.path&&list.items[i].subsong==item.subsong;};if(same(list.current))return list.current;for(int i=0;i<(int)list.items.size();++i)if(same(i))return i;return -1;}
inline picojson::value encode(const Bundle& bundle){
 picojson::object root;root["format"]=picojson::value("foo_dj.session");root["version"]=picojson::value(1.);picojson::array lists;
 for(auto& list:bundle.lists){picojson::object o;o["items"]=dj_queue::encode(list.items);o["current"]=picojson::value(double(list.current));lists.emplace_back(o);}root["playlists"]=picojson::value(lists);
 if(bundle.full){picojson::array decks;for(auto& d:bundle.decks){auto o=trackValue(d.item).get<picojson::object>();o["position"]=picojson::value(d.position);o["volume"]=picojson::value(d.volume);o["speed"]=picojson::value(d.speed);o["range"]=picojson::value(double(d.range));o["transpose"]=picojson::value(double(d.transpose));o["fixedPitch"]=picojson::value(d.fixed);o["snap"]=picojson::value(d.snap);o["loop"]=loopValue(d.loop);if(d.manual)o["gridPhase"]=picojson::value(d.phase);decks.emplace_back(o);}root["decks"]=picojson::value(decks);
  picojson::object mixer;mixer["crossfade"]=picojson::value(bundle.crossfade);mixer["fadeSeconds"]=picojson::value(bundle.fade);mixer["linear"]=picojson::value(bundle.linear);root["mixer"]=picojson::value(mixer);
  picojson::array markers;for(auto& m:bundle.markers){auto o=trackValue(m.item).get<picojson::object>();o["loop"]=loopValue(m.loop);if(m.manual)o["gridPhase"]=picojson::value(m.phase);markers.emplace_back(o);}root["markers"]=picojson::value(markers);
 }return picojson::value(root);
}
inline Bundle decode(const picojson::value& root){
 require(root.is<picojson::object>()&&root.get("format").is<std::string>()&&root.get("format").get<std::string>()=="foo_dj.session","Not a DJ Decks session file");require(integer(root.get("version"),1,1)==1,"Unsupported session version");Bundle out;
 const auto& lists=root.get("playlists");require(lists.is<picojson::array>()&&lists.get<picojson::array>().size()==2,"Exactly two playlists required");size_t bytes=0;
 for(int i=0;i<2;++i){const auto& list=lists.get<picojson::array>()[i];require(list.is<picojson::object>(),"Invalid playlist");const auto& items=list.get("items");require(items.is<picojson::array>()&&items.get<picojson::array>().size()<=10000,"Playlist limit exceeded");for(auto& item:items.get<picojson::array>())out.lists[i].items.push_back(track(item));out.lists[i].current=integer(list.get("current"),-1,(int)out.lists[i].items.size()-1);bytes+=items.serialize().size();}
 require(bytes<=3*1024*1024,"Combined playlist storage limit exceeded");
 const auto& decks=root.get("decks");if(decks.is<picojson::null>())return out;require(decks.is<picojson::array>()&&decks.get<picojson::array>().size()==2,"Exactly two decks required");out.full=true;
 for(int i=0;i<2;++i){const auto& value=decks.get<picojson::array>()[i];auto& d=out.decks[i];d.item=track(value,true);d.position=number(value.get("position"),0,1e12);d.volume=number(value.get("volume"),0,1);d.range=integer(value.get("range"),8,50);require(d.range==8||d.range==16||d.range==50,"Invalid speed range");d.speed=number(value.get("speed"),1-d.range/100.,1+d.range/100.);d.transpose=integer(value.get("transpose"),-12,12);d.fixed=boolean(value.get("fixedPitch"));d.snap=boolean(value.get("snap"));d.loop=loopValue(value.get("loop"));if(!value.get("gridPhase").is<picojson::null>()){d.phase=number(value.get("gridPhase"),0,1e12);d.manual=true;}}
 auto& mixer=root.get("mixer");require(mixer.is<picojson::object>(),"Missing mixer");out.crossfade=number(mixer.get("crossfade"),0,1);out.fade=number(mixer.get("fadeSeconds"),0,30);out.linear=boolean(mixer.get("linear"));
 std::set<std::pair<std::string,unsigned>> referenced;for(auto& list:out.lists)for(auto& item:list.items)referenced.emplace(item.path,item.subsong);for(auto& deck:out.decks)if(!deck.item.path.empty())referenced.emplace(deck.item.path,deck.item.subsong);
 const auto& markers=root.get("markers");require(markers.is<picojson::array>()&&markers.get<picojson::array>().size()<=20002,"Invalid markers");std::set<std::pair<std::string,unsigned>> seen;
 for(auto& value:markers.get<picojson::array>()){Marker m;m.item=track(value);require(referenced.count({m.item.path,m.item.subsong})!=0,"Marker outside the saved playlists/decks");require(seen.emplace(m.item.path,m.item.subsong).second,"Duplicate marker track");m.loop=loopValue(value.get("loop"));if(!value.get("gridPhase").is<picojson::null>()){m.phase=number(value.get("gridPhase"),0,1e12);m.manual=true;}out.markers.push_back(m);}return out;
}
inline std::string serialize(const Bundle& bundle){auto text=encode(bundle).serialize(true);require(text.size()<=maxBytes,"Session file too large");decode(encode(bundle));return text;}
inline Bundle parse(std::string text){require(text.size()<=maxBytes,"Session file too large");if(text.size()>=3&&(unsigned char)text[0]==0xef&&(unsigned char)text[1]==0xbb&&(unsigned char)text[2]==0xbf)text.erase(0,3);picojson::value value;require(picojson::parse(value,text).empty(),"Malformed session JSON");return decode(value);}
}
