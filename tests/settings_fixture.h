#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <atlbase.h>
#include <atltypes.h>
#include "../third_party/WTL/Include/atlapp.h"
#include <atlwin.h>
#include "../third_party/WTL/Include/atlcrack.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <iostream>
using namespace ATL;
using namespace WTL;
CAppModule _Module;

#include <atomic>
#include <map>
#include "../sdk/foobar2000/foo_dj/settings_window.h"
#include "../sdk/foobar2000/foo_dj/dj_skin.h"
#include "../sdk/foobar2000/foo_dj/queue_format.h"
#include "../sdk/foobar2000/foo_dj/preview_plan.h"
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"ole32.lib")
namespace pfc::stringcvt {struct string_wide_from_utf8{std::wstring value;string_wide_from_utf8(const char* text){int n=MultiByteToWideChar(CP_UTF8,0,text,-1,nullptr,0);value.resize(n);MultiByteToWideChar(CP_UTF8,0,text,-1,value.data(),n);if(n)value.resize(n-1);}const wchar_t* get_ptr()const{return value.c_str();}};}
template<class T> struct Config{T value{};T get()const{return value;}void operator=(T v){value=v;}};
struct Text {std::string value;const char* get_ptr()const{return value.c_str();}};
struct TextConfig {Text value;Text get()const{return value;}};
inline Config<int> cfg_outputRate{0};
inline Config<int> cfg_bpmProvider{0};
inline Config<int> cfg_masterLimiter{1},cfg_masterCeiling{-10};
inline Config<GUID> cfg_cueBackend,cfg_cueDevice;
inline Config<int> cfg_cueVolume{25},cfg_previewMode,cfg_queueNumbers,cfg_trimHead,cfg_trimTail,cfg_trimDb{-60},cfg_loopSnapA,cfg_loopSnapB;
inline TextConfig cfg_queueFormat{{dj_queue_format::presets[0]}};
inline std::atomic<bool> g_english{false},g_scrobble{false};inline std::atomic<int> g_replaygain{0};
struct Core {GUID m_output{},m_device{};};
struct output_manager {Core core;static output_manager* get(){static output_manager m;return &m;}Core getCoreConfig(){return core;}};
struct CDjPanel {
 struct SettingsDevice{std::wstring name;GUID backend,device;};std::vector<SettingsDevice> m_settingsDevices;std::wstring m_settingsDeviceError;
 struct Output {std::string lastError(){return {};}}m_cueOutput;
 std::atomic<bool> m_cue[2]={{false},{false}};std::atomic<double> m_cueVolume{.25};double m_fadeSeconds=5;std::atomic<bool> m_linearFade{false};bool m_loopSnap[2]={};
 std::atomic<unsigned> m_outputRatePreference{0};std::atomic<bool> m_limitEnabled{true};std::atomic<double> m_limitCeiling{-1};
 DjSettingsWindow m_settings;std::vector<int> commands;int stopped=0,configured=0,refreshed=0,trimmed=0,saved=0,painted=0;
 COLORREF DeckAccent(int){return RGB(0,194,211);}void Invalidate(){++painted;}void StopPreview(){++stopped;}void ConfigureCue(){++configured;}void RefreshSettingsDevices(){++refreshed;}void RefreshTrimPreparations(){++trimmed;}void SaveSession(){++saved;}
 void SettingsCommand(int id){commands.push_back(id);if(id==1||id==2)g_english=id==2;if(id==10||id==11)dj_skin::flat=id==11;if(id>=100&&id<108)dj_skin::colorIndex[0]=id-100;if(id>=120&&id<128)dj_skin::colorIndex[1]=id-120;if(id>=210&&id<213)g_replaygain=id-210;if(id>=300&&id<304)cfg_queueFormat.value.value=dj_queue_format::presets[id-300];if(id==305)cfg_queueNumbers=cfg_queueNumbers.get()?0:1;if(id==220)g_scrobble=!g_scrobble.load();}
 std::vector<dj_settings::Item> SettingsValues(int);
 void SettingsChanged(int,int);
 int distributed=0;void DistributeActivePlaylist(){++distributed;}
 int exported=-1,imported=-1;void ExportBundle(bool full){exported=full?1:0;}void ImportBundle(bool recovery){imported=recovery?1:0;}
 void SetLoopSnap(int deck,bool enabled){m_loopSnap[deck]=enabled;if(deck)cfg_loopSnapB=enabled?1:0;else cfg_loopSnapA=enabled?1:0;}
};
