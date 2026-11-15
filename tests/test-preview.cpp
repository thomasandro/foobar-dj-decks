#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/preview_plan.h"
struct Abort{std::atomic<bool> stopped{false};void set(){stopped=true;}bool is_aborting(){return stopped.load();}};
template<class F>bool until(F done){auto start=GetTickCount64();while(GetTickCount64()-start<10000){if(done())return true;Sleep(1);}return done();}
int main(){
    using namespace dj_preview;
    auto p=plan(100,0);assert(p.size()==1&&p[0].start==0&&p[0].seconds==8);
    p=plan(100,1);assert(p[0].start==0&&p[0].seconds==15);
    p=plan(100,2);assert(p[0].start==46&&p[0].seconds==8);
    p=plan(100,3);assert(p[0].start==42.5&&p[0].seconds==15);
    p=plan(100,4);assert(p.size()==2&&p[0].start==0&&p[1].start==46);
    p=plan(10,4);assert(p.size()==2&&p[0].seconds==8&&p[1].start==8&&p[1].seconds==2);
    p=plan(3,4);assert(p.size()==1&&p[0].seconds==3);p=plan(0,0);assert(p[0].seconds==8);
    bool threw=false;try{plan(0,2);}catch(...){threw=true;}assert(threw);
    p=planAt(100,0,30);assert(p.size()==1&&p[0].start==30&&p[0].seconds==8);
    p=planAt(100,3,30);assert(p[0].start==30&&p[0].seconds==15);
    p=planAt(100,4,30);assert(p.size()==1&&p[0].seconds==16);
    p=planAt(100,0,98);assert(p[0].start==98&&p[0].seconds==2);
    p=planAt(100,0,100);assert(p[0].start<100&&p[0].seconds>0&&p[0].seconds<.002);
    p=planAt(100,2,-1);assert(p[0].start==46);
    std::atomic<int> opened{0},closed{0},result{0};
    Runner<int,Abort> runner([&](const int& request,Abort& abort){
        ++opened;struct Close{std::atomic<int>& count;~Close(){++count;}} close{closed};
        if(request<0)throw std::runtime_error("decode failed");
        for(int n=0;n<100;++n){if(abort.is_aborting())return;Sleep(1);}result=request;
    });
    runner.start(1);assert(until([&]{return opened==1;}));runner.start(2);assert(until([&]{return !runner.active();}));assert(result==2&&opened==closed);
    runner.start(3);assert(until([&]{return opened>closed;}));runner.cancel(true);assert(opened==closed&&!runner.active());
    runner.start(-1);assert(until([&]{return !runner.active();}));assert(runner.lastError()=="decode failed"&&opened==closed);
    runner.start(4);assert(until([&]{return !runner.active();}));assert(result==4&&runner.lastError().empty());
    runner.stop();assert(opened==closed);
    std::cout<<"Preview planning and actual latest-request runner: five modes, short/unknown lengths, replacement, cancellation, decoder release, failure and recovery passed (mock decoder)\n";
}
