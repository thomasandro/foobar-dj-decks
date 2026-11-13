#include "../sdk/foobar2000/foo_dj/musical_key.h"
#include <cassert>
#include <iostream>
int main() {
    const wchar_t* majors[]={L"C",L"Db",L"D",L"Eb",L"E",L"F",L"F#",L"G",L"Ab",L"A",L"Bb",L"B"};
    const int major[]={8,3,10,5,12,7,2,9,4,11,6,1};
    const int minor[]={5,12,7,2,9,4,11,6,1,8,3,10};
    for(int i=0;i<12;++i) {
        assert(dj_key::camelot(majors[i])==std::to_wstring(major[i])+L"B");
        assert(dj_key::camelot(std::wstring(majors[i])+L"m")==std::to_wstring(minor[i])+L"A");
        for(auto mode:{L"A",L"B"}) { auto code=std::to_wstring(i+1)+mode; assert(dj_key::camelot(code)==code); }
    }
    assert(dj_key::camelot(L" C major ")==L"8B");
    assert(dj_key::camelot(L"AM")==L"11B");
    assert(dj_key::camelot(L"A minor")==L"8A");
    assert(dj_key::camelot(L"G\u266fm")==L"1A");
    assert(dj_key::camelot(L"A\u266d minor")==L"1A");
    assert(dj_key::camelot(L"1m")==L"8A");
    assert(dj_key::camelot(L"12d")==L"7B");
    for(auto bad:{L"13A",L"0B",L"C dorian",L"unknown",L"Am/C",L"999999A",L""}) assert(dj_key::camelot(bad).empty());
    assert(dj_key::display(L"  unknown  ")==L"unknown");
    assert(dj_key::display(L"   ").empty());
    assert(dj_key::label(L"8A")==L"8A (Am)");
    assert(dj_key::label(L"C")==L"8B (C)");
    assert(dj_key::label(L"Dm")==L"7A (Dm)");
    assert(dj_key::label(L"unknown")==L"unknown");
    using C=dj_key::Compatibility;
    for(int a=1;a<=12;++a) for(int b=1;b<=12;++b) for(auto x:{L"A",L"B"}) for(auto y:{L"A",L"B"}) {
        auto left=std::to_wstring(a)+x, right=std::to_wstring(b)+y;
        const bool expected=a==b || (std::wstring(x)==y && ((a-b+12)%12==1 || (b-a+12)%12==1));
        assert(dj_key::compatibility(left,right)==(expected?C::usual:C::other));
        assert(dj_key::compatibility(left,right,false)==C::unknown);
        assert(!dj_key::standard(left).empty());
        assert(dj_key::camelot(dj_key::standard(left))==left);
    }
    assert(dj_key::compatibility(L"",L"7A")==C::unknown);
    assert(dj_key::compatibility(L"noise",L"7A")==C::unknown);
    assert(dj_key::transpose(L"Am",1)==L"3A");
    assert(dj_key::transpose(L"C",-1)==L"1B");
    assert(dj_key::transpose(L"8A",12)==L"8A");
    for(int n=-12;n<=12;++n) for(int k=1;k<=12;++k) {
        auto key=std::to_wstring(k)+L"A";
        assert(dj_key::transpose(dj_key::transpose(key,n),-n)==key);
    }
    std::cout<<"Musical key conversion passed\n";
}
