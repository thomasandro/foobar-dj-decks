#define NOMINMAX
#include <windows.h>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include <limits>
using t_size=size_t;
namespace titleformat_inputtypes {inline const GUID meta={};}
class titleformat_hook_function_params{};
class titleformat_text_out{public:virtual void write(const GUID&,const char*,t_size=SIZE_MAX)=0;};
class titleformat_hook{public:virtual bool process_field(titleformat_text_out*,const char*,t_size,bool&)=0;virtual bool process_function(titleformat_text_out*,const char*,t_size,titleformat_hook_function_params*,bool&)=0;};
#include "../sdk/foobar2000/foo_dj/queue_duration_hook.h"
struct Out:titleformat_text_out {std::string text;void write(const GUID&,const char* p,t_size n=SIZE_MAX)override{text.append(p,n==SIZE_MAX?std::strlen(p):n);}};
int main(){
    assert(dj_queue_format::duration(1027)=="17:07");assert(dj_queue_format::duration(238)=="3:58");assert(dj_queue_format::duration(3605)=="1:00:05");
    assert(dj_queue_format::duration(0)=="?"&&dj_queue_format::duration(-1)=="?"&&dj_queue_format::duration(std::numeric_limits<double>::quiet_NaN())=="?");
    Out out;bool found=false;DjQueueDurationHook hook(1027);assert(hook.process_field(&out,"length",6,found)&&found&&out.text=="17:07");
    out.text.clear();assert(hook.process_field(&out,"LENGTH",6,found)&&found&&out.text=="17:07");
    out.text.clear();assert(hook.process_field(&out,"length_seconds",14,found)&&found&&out.text=="1027.000");
    assert(!hook.process_field(&out,"artist",6,found));
    out.text.clear();assert(hook.process_field(&out,"length",SIZE_MAX,found)&&found&&out.text=="17:07");
    assert(!hook.process_field(&out,"artist",SIZE_MAX,found));
    DjQueueDurationHook unknown(0);out.text.clear();assert(unknown.process_field(&out,"length",6,found)&&!found&&out.text=="?");
    std::cout<<"Actual queue duration hook: minute/hour labels, length and length_seconds, case, unknown duration and unrelated field preservation passed\n";
}
