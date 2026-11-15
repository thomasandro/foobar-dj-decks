#pragma once
#include "third_party/foo_scrobble/ScrobbleService.h"
#include "scrobble_detection.h"
// The upstream service has no version negotiation. Restrict the adapter to the
// 1.6 family whose Track/service layout was inspected; fail closed otherwise.
inline const GUID foo_scrobble::ScrobbleService::class_guid={0x92f5e908,0x14d5,0x4334,{0x85,0x92,0x30,0x6c,0xff,0x60,0xf6,0x33}};
namespace dj_scrobble {
struct Probe{foo_scrobble::ScrobbleService::ptr api;std::string module,version;bool present=false,supported=false;
    std::wstring message(bool en)const{
        std::string s;if(!present)s=en?"foo_scrobble was not found in the component registry.":"foo_scrobble absent du registre des composants.";
        else if(!supported)s=en?"Unsupported foo_scrobble version: ":"Version foo_scrobble non prise en charge : ";
        else if(!api.is_valid())s=en?"foo_scrobble detected, but its ScrobbleService is unavailable: ":"foo_scrobble détecté, mais son ScrobbleService est indisponible : ";
        else s=en?"foo_scrobble service detected: ":"Service foo_scrobble détecté : ";
        if(present)s+=version+" (module: "+module+")";
        return pfc::stringcvt::string_wide_from_utf8(s.c_str()).get_ptr();
    }
};
inline Probe probe(){
    Probe result;service_enum_t<componentversion> e;componentversion::ptr version;
    while(e.next(version)){pfc::string8 file,name,v;version->get_file_name(file);version->get_component_name(name);if(!componentMatches(file.c_str(),name.c_str()))continue;version->get_component_version(v);result.present=true;result.module=moduleName(file.c_str());result.version=trimmed(v.c_str());result.supported=supportedVersion(result.version);if(result.supported)break;}
    if(result.supported){service_enum_t<foo_scrobble::ScrobbleService> services;services.next(result.api);}return result;
}
inline foo_scrobble::ScrobbleService::ptr service(){return probe().api;}
inline bool hostEnabled(){
    const GUID toggle={0x7c90acef,0x2858,0x49ad,{0xb6,0x14,0xf3,0xfd,0x4c,0x84,0x81,0x49}};
    service_enum_t<mainmenu_commands> items;mainmenu_commands::ptr item;
    while(items.next(item))for(t_uint32 n=0;n<item->get_command_count();++n)if(item->get_command(n)==toggle){pfc::string8 label;t_uint32 flags=0;return item->get_display(n,label,flags)&&(flags&mainmenu_commands::flag_checked)!=0;}
    return false;
}
inline foo_scrobble::Track track(const file_info& info,double duration,long long start){
    foo_scrobble::Track t;t.Timestamp=foo_scrobble::unix_clock::from_time_t((time_t)start);t.Duration=std::chrono::duration<double>(duration);
    auto tag=[&](const char* name){auto p=info.meta_get(name,0);return p?p:"";};
    t.Artist=tag("artist");t.Title=tag("title");t.AlbumArtist=tag("album artist");t.Album=tag("album");t.TrackNumber=tag("tracknumber");t.MusicBrainzId=tag("musicbrainz_trackid");return t;
}
}
