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
 for(double range : {8.0,16.0,50.0}) {
   const double r=range/100;
   assert(match(100,100*(1-r),1,speed,range)==Match::ok && std::abs(speed-(1-r))<1e-9);
   assert(match(100,100*(1+r),1,speed,range)==Match::ok && std::abs(speed-(1+r))<1e-9);
   speed=1;
   assert(match(100,100*(1+r)+1,1,speed,range)==Match::outside && speed==1);
   assert(match(100,100*(1-r)-1,1,speed,range)==Match::outside && speed==1);
   assert(clamp(2,range)==1+r && clamp(0,range)==1-r);
 }
 assert(match(100,140,1,speed,50)==Match::ok && std::abs(speed-1.4)<1e-9);
 assert(match(100,110,1,speed,8)==Match::outside);
 assert(match(100,100,1.4,speed,50)==Match::ok);
 std::cout<<"Tempo matching tests passed: both directions, adjusted reference, range, invalid tags\n";
}
