#include "../sdk/foobar2000/foo_dj/tempo_match.h"
#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>
int main(){
 using namespace dj_tempo; double speed=1;
 assert(match(120,126,1,speed)==Match::ok && std::abs(speed-1.05)<1e-12);
 assert(match(120,120,1.1,speed)==Match::ok && std::abs(speed-1.1)<1e-12);
 assert(match(126,120,1,speed)==Match::ok && std::abs(126*speed-120)<1e-9);
 assert(match(100,84,1,speed)==Match::ok && speed==0.84);
 assert(match(100,116,1,speed)==Match::ok && speed==1.16);
 speed=1.03; assert(match(100,140,1,speed)==Match::outside && speed==1.03);
 assert(match(100,60,1,speed)==Match::outside && speed==1.03);
 assert(match(0,120,1,speed)==Match::missing && speed==1.03);
 assert(match(120,std::numeric_limits<double>::quiet_NaN(),1,speed)==Match::missing);
 std::cout<<"Tempo matching tests passed: both directions, adjusted reference, range, invalid tags\n";
}
