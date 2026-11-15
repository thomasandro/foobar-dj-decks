#define NOMINMAX
#include "../sdk/foobar2000/foo_dj/deck_queue.h"
#include "../sdk/foobar2000/foo_dj/queue_format.h"
#include <cassert>
#include <iostream>
using namespace dj_queue;
int main(){
    assert(std::string(dj_queue_format::presets[0])=="%artist% - %title% '('%length%')'");
    assert(dj_queue_format::singleLine(L"artist\ntitle\r\tlength")==L"artist title  length");
    for(bool moving:{false,true})for(unsigned mask=1;mask<32;++mask)for(size_t boundary=0;boundary<=3;++boundary){
        std::vector<Item> from;for(unsigned n=0;n<5;++n)from.push_back({std::to_string(n),n});std::vector<Item> to={{"x",9},{"x",10},{"z",11}};
        std::vector<size_t> selected,inserted;for(unsigned n=0;n<5;++n)if(mask&(1<<n))selected.push_back(n);
        int a=3,b=1;assert(transfer(from,a,to,b,selected,boundary,moving,inserted));assert(inserted.size()==selected.size());
        assert(to.size()==3+selected.size()&&from.size()==(moving?5-selected.size():5));
        for(size_t i=0;i<selected.size();++i){assert(to[boundary+i].path==std::to_string(selected[i]));assert(to[boundary+i].subsong==selected[i]);assert(inserted[i]==boundary+i);}
        assert(b==(boundary<=1?1+(int)selected.size():1));
        if(moving){if(mask&(1<<3))assert(a==-1);else {int expected=3;for(auto n:selected)if(n<3)--expected;assert(a==expected);}}
        else assert(a==3);
    }
    {std::vector<Item> a={{"same",1},{"same",2}},b;int ca=0,cb=-1;std::vector<size_t> inserted;
     assert(transfer(a,ca,b,cb,{1,1,99},999,false,inserted)&&b.size()==1&&b[0].subsong==2&&a.size()==2);
     assert(!transfer(a,ca,b,cb,{99},0,true,inserted));assert(!transfer(a,ca,a,ca,{0},0,true,inserted));}
    {std::vector<Item> a={{"a",0}},b(10000,Item{"b",0});int ca=0,cb=0;std::vector<size_t> inserted;
     assert(!transfer(a,ca,b,cb,{0},0,true,inserted));assert(a.size()==1&&b.size()==10000&&ca==0&&cb==0);}
    {std::vector<Item> a={{std::string(1600000,'a'),0}},b={{"b",0}};int ca=0,cb=0;std::vector<size_t> inserted;
     assert(!transfer(a,ca,b,cb,{0},0,false,inserted));assert(a.size()==1&&b.size()==1&&ca==0&&cb==0);
     assert(transfer(a,ca,b,cb,{0},0,true,inserted));assert(a.empty()&&b.size()==2&&ca==-1&&cb==1);}
    std::cout<<"Queue transfer: 248 copy/move cases, source order/subsongs, active occurrences, duplicates, invalid selections, entry/byte-limit rollback and default display format passed\n";
}
