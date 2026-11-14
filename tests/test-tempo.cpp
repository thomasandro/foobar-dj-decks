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
 assert(match(100,40,1,speed)==Match::outside && speed==1.03);
 assert(match(0,120,1,speed)==Match::missing && speed==1.03);
 assert(match(120,std::numeric_limits<double>::quiet_NaN(),1,speed)==Match::missing);
 for(double range : {8.0,16.0,50.0}) {
   const double r=range/100;
   speed=1-r;assert(match(100,100*(1-r),1,speed,range)==Match::ok && std::abs(speed-(1-r))<1e-9);
   speed=1+r;assert(match(100,100*(1+r),1,speed,range)==Match::ok && std::abs(speed-(1+r))<1e-9);
   speed=1;
   assert(match(100,200*(1+r)+1,1,speed,range)==Match::outside && speed==1);
   assert(match(100,50*(1-r)-1,1,speed,range)==Match::outside && speed==1);
   assert(clamp(2,range)==1+r && clamp(0,range)==1-r);
 }
 speed=1;double factor=0;
 assert(match(100,140,1,speed,50,&factor)==Match::ok && std::abs(speed-.7)<1e-9 && factor==.5);
 speed=1.3;assert(match(100,140,1,speed,50,&factor)==Match::ok && std::abs(speed-1.4)<1e-9&&factor==1);
 speed=1.05;assert(match(100,140,1,speed,50,&factor)==Match::ok && std::abs(speed-1.4)<1e-9&&factor==1); // exact wins tie
 for(double range:{8.,16.,50.}){
   speed=1;assert(match(86,172,1,speed,range,&factor)==Match::ok&&speed==1&&factor==.5);
   speed=1;assert(match(172,86,1,speed,range,&factor)==Match::ok&&speed==1&&factor==2);
   speed=1;assert(match(86,174,1,speed,range,&factor)==Match::ok&&std::abs(speed-87./86)<1e-12&&factor==.5);
 }
 speed=1;assert(match(100,180,1.1,speed,8,&factor)==Match::ok&&std::abs(speed-.99)<1e-12&&factor==.5);
 speed=1;factor=7;assert(match(100,1000,1,speed,50,&factor)==Match::outside&&speed==1&&factor==7);
 speed=1;assert(match(100,100,std::numeric_limits<double>::infinity(),speed)==Match::missing);
 assert(match(100,110,1,speed,8)==Match::outside);
 assert(match(100,100,1.4,speed,50)==Match::ok);
 std::cout<<"Tempo matching passed: exact/half/double, nearest current speed, exact ties, both directions, adjusted reference, range and invalid tags\n";
}
