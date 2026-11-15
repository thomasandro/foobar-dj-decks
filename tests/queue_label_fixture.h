#define NOMINMAX
#include <windows.h>
#include <string>
#include <memory>
#include <cstring>
#include <cassert>
#include <iostream>
using t_size=size_t;
namespace titleformat_inputtypes{inline const GUID meta={};}
class titleformat_hook_function_params{};
class titleformat_text_out{public:virtual void write(const GUID&,const char*,t_size=SIZE_MAX)=0;};
class titleformat_hook{public:virtual bool process_field(titleformat_text_out*,const char*,t_size,bool&)=0;virtual bool process_function(titleformat_text_out*,const char*,t_size,titleformat_hook_function_params*,bool&)=0;};
#include "../sdk/foobar2000/foo_dj/queue_duration_hook.h"
namespace pfc{
struct string8{std::string value;operator const char*()const{return value.c_str();}bool is_empty()const{return value.empty();}const char* get_ptr()const{return value.c_str();}};
namespace stringcvt{struct string_wide_from_utf8{std::wstring value;explicit string_wide_from_utf8(const char* p){auto n=MultiByteToWideChar(CP_UTF8,0,p,-1,nullptr,0);value.resize(n);MultiByteToWideChar(CP_UTF8,0,p,-1,value.data(),n);value.resize(n-1);}const wchar_t* get_ptr()const{return value.c_str();}};}
}
struct Format{std::string script;};struct FormatPtr:std::shared_ptr<Format>{using std::shared_ptr<Format>::operator=;bool is_valid()const{return bool(*this);}};
struct titleformat_compiler{static titleformat_compiler* get(){static titleformat_compiler c;return &c;}bool compile(FormatPtr& out,const char* text){out=std::make_shared<Format>();out->script=text;return true;}void compile_force(FormatPtr& out,const char* text){compile(out,text);}};
struct Config{std::string script=dj_queue_format::presets[0];pfc::string8 get(){return {script};}};inline Config cfg_queueFormat;
struct file_info_impl{};
struct Handle{double seconds=1027;void format_title(titleformat_hook* hook,pfc::string8& out,const FormatPtr& f,void*){
    out.value="Artiste é - Titre";if(f->script.find("%length%")!=std::string::npos){struct Out:titleformat_text_out{std::string text;void write(const GUID&,const char* p,t_size n=SIZE_MAX)override{text.append(p,n==SIZE_MAX?std::strlen(p):n);}} duration;bool found=false;
        assert(hook->process_field(&duration,"length",SIZE_MAX,found));out.value+=" ("+duration.text+")";
    }
}};
struct metadb{std::shared_ptr<Handle> handle=std::make_shared<Handle>();static metadb* get(){static metadb m;return &m;}std::shared_ptr<Handle> handle_create(const char*,unsigned){return handle;}};
namespace dj_queue{struct Item{std::string path;unsigned subsong=0;};}
class CDjPanel{public:FormatPtr m_queueFormat;std::string m_queueFormatText;std::wstring m_queueFormatError;double QueueDuration(const dj_queue::Item&,std::shared_ptr<Handle> h){return h->seconds;}std::wstring QueueLabel(const dj_queue::Item&);};
