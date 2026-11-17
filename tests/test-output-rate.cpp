#include "../sdk/foobar2000/foo_dj/output_rate.h"
#include <cassert>
#include <iostream>
int main(){using dj_output::rate;assert(rate(0,0,44100)==44100);assert(rate(0,0,48000)==48000);assert(rate(0,0,0)==44100);assert(rate(0,44100,96000)==44100);assert(rate(48000,44100,44100)==48000);assert(rate(44100,48000,48000)==44100);assert(rate(0,123,1)==44100);assert(rate(0,96000,44100)==96000);std::cout<<"Output-rate policy: forced device priority, first-track 44.1/48, explicit preference and safe 44.1 fallback passed\n";}
