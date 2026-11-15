#include "../sdk/foobar2000/foo_dj/listening_policy.h"
#include <cassert>
#include <iostream>
#include <limits>
using dj_listening::Clock;
int main(){
    Clock c;c.consume(48000*50,48000,1,false,1000);assert(c.seconds==0&&!c.started);
    c.consume(48000*50,48000,0,true,1000);assert(c.seconds==0);
    c.consume(48000*50,48000,.001,true,1000);assert(c.seconds==0);
    c.consume(48000*50,48000,1,true,1000);assert(c.seconds==50&&c.started==1000&&!c.eligible(200));
    // Position/tempo are not inputs: 50 output seconds earn exactly 50 seconds.
    c.consume(48000*50,48000,.5,true,1200);assert(c.seconds==100&&c.started==1000&&c.eligible(200));
    c.submitted=true;assert(!c.eligible(200));c.discardUnsubmitted();assert(c.submitted&&c.seconds==100);
    Clock d;d.consume(44100*239,44100,.01,true,2000);assert(!d.eligible(1200));d.consume(44100,44100,.01,true,2300);assert(d.eligible(1200));
    assert(!d.eligible(30)&&!d.eligible(0)&&!d.eligible(std::numeric_limits<double>::quiet_NaN()));
    d.discardUnsubmitted();assert(d.seconds==0&&d.started==0);d.consume(48000*16,48000,1,true,3000);assert(d.eligible(31));
    Clock a,b;a.consume(48000*100,48000,1,true,4000);b.consume(48000*100,48000,0,true,4000);assert(a.eligible(200)&&!b.eligible(200));
    using dj_listening::gain;assert(gain(0,.5,.25)==1&&gain(1,.5,.25)==.5&&gain(2,.5,.25)==.25);
    assert(gain(1,0,.25)==1&&gain(2,1,std::numeric_limits<double>::infinity())==1&&gain(99,.5,.25)==1);
    std::cout<<"Listening: opt-in, inaudible decks, output-time thresholds, long/short tracks, one submission, disable/reset and independent decks passed; ReplayGain mode/sanitization passed\n";
}
