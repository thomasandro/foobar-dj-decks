#define NOMINMAX
#include <windows.h>
#include <atomic>
#include <cassert>
#include <vector>
#include <utility>
#include <cmath>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/midi_control.h"
#include "../sdk/foobar2000/foo_dj/tempo_match.h"
struct Config {GUID value{};GUID get(){return value;}} cfg_cueBackend;
struct CDjPanel {
 std::atomic<float> m_crossfade{.5f},m_volume[2]={{1},{1}};
 std::atomic<double> m_speed[2]={{1},{1}};std::atomic<int> m_speedRange[2]={{16},{50}};
 std::atomic<bool> m_cue[2]={{false},{false}};bool m_dragDirty=false;int cancelled=0,stopped=0,configured=0;
 std::vector<std::pair<int,int>> calls;
 double MidiCurrent(int);void MidiApply(int,double);
 void CancelAutomation(){++cancelled;}void TogglePlay(int d){calls.push_back({0,d});}void PlayBoth(){calls.push_back({1,0});}
 void SeekDeck(int d,double p){assert(p==0);calls.push_back({2,d});}void ReleaseForTags(int d){calls.push_back({3,d});}
 void StopPreview(){++stopped;}void ConfigureCue(){++configured;}void EditLoop(int d,int a){calls.push_back({4+d,a});}
 void MatchTempo(int d){calls.push_back({6,d});}void MatchKey(int d){calls.push_back({7,d});}
};
#include "../build/tests/midi-apply.inc"
int main(){using namespace dj_midi;CDjPanel p;
 assert(p.MidiCurrent(crossfade)==.5&&p.MidiCurrent(volumeA)==1&&p.MidiCurrent(tempoB)==.5);
 p.MidiApply(crossfade,0);assert(p.m_crossfade==0&&p.cancelled==1&&p.m_dragDirty);
 p.MidiApply(volumeA,.25);p.MidiApply(volumeB,.75);assert(p.m_volume[0]==.25&&p.m_volume[1]==.75);
 p.MidiApply(tempoA,0);p.MidiApply(tempoB,1);assert(std::abs(p.m_speed[0]-.84)<1e-9&&p.m_speed[1]==1.5);
 assert(std::abs(p.MidiCurrent(tempoA))<1e-9&&p.MidiCurrent(tempoB)==1);
 for(int a=playA;a<count;++a)p.MidiApply(a,1);
 std::vector<std::pair<int,int>> expected={{0,0},{0,1},{1,0},{2,0},{2,1},{3,0},{3,1},{4,0},{4,1},{4,2},{5,0},{5,1},{5,2},{6,0},{6,1},{7,0},{7,1}};
 assert(p.calls==expected&&!p.m_cue[0]&&!p.m_cue[1]);cfg_cueBackend.value.Data1=1;p.MidiApply(cueA,1);p.MidiApply(cueB,1);assert(p.m_cue[0]&&p.m_cue[1]&&p.stopped==2&&p.configured==2);
 std::cout<<"Actual MIDI dispatcher: 24 commands, deck isolation, selected tempo ranges, transport/loop/BPM/key delegation, cue configuration guard and mixer takeover values passed (mock panel, no host/device).\n";
}
