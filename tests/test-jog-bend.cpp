#include "../sdk/foobar2000/foo_dj/jog_bend.h"
#include <cassert>
#include <limits>
#include <iostream>
int main(){dj_jog::Bend a,b;assert(a.value(0)==0);a.pulse(1,2,1000);assert(a.value(1000)==.005&&a.value(1120)==.005);assert(std::abs(a.value(1210)-.0025)<1e-12);assert(a.value(1300)==0&&a.value(100000)==0);
 a.pulse(16,2,2000);assert(a.value(2000)==.02);a.pulse(-16,2,2000);assert(a.value(2000)==-.02&&b.value(2000)==0);a.clear();assert(a.value(2000)==0);
 for(int n=0;n<5;++n){a.clear();a.pulse(1000,n,0);const double limits[]={.005,.01,.02,.04,.08};assert(a.value(0)==limits[n]);a.pulse(std::numeric_limits<double>::quiet_NaN(),n,1);assert(a.value(1)==limits[n]);}
 a.clear();a.pulse(4,2,(1ULL<<48)-100);assert(a.value(20)==.02&&a.value(200)==0);a.clear();a.pulse(1,2,1000);a.pulse(-1,2,1000);assert(a.value(1000)==0);
 std::cout<<"Jog bend: bounded signed impulses, hold/decay/expiry, reversal, independent decks, invalid inputs and clock wrap passed.\n";
}
