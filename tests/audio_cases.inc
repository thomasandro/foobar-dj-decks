int main(){for(bool fixed : {false,true}) for(double speed : {0.84,1.0,1.16}){
 Deck d;abort_callback abort;std::vector<float> result;
 for(int pass=0;pass<2000;++pass){FillDeck(d,1024,48000,abort,speed,fixed);size_t n=(std::min)((size_t)1024,d.pendingFrames());result.insert(result.end(),d.pending.begin()+d.pendingPos,d.pending.begin()+d.pendingPos+n*2);d.pendingPos+=n*2;if(d.pendingPos==d.pending.size()){d.pending.clear();d.pendingPos=0;}if(d.eof&&d.pendingFrames()==0)break;}
 double duration=result.size()/2.0/48000;assert(std::abs(duration-3/speed)<0.025);
 size_t first=24000,last=result.size()/2-24000;unsigned crossings=0;for(size_t n=first+1;n<last;++n)if(result[2*(n-1)]<=0&&result[2*n]>0)++crossings;
 double hz=crossings*48000.0/(last-first),expected=440*(fixed?1:speed);assert(std::abs(hz-expected)<2.0);
 std::cout<<"fixed="<<fixed<<" speed="<<speed<<" duration="<<duration<<" frequency="<<hz<<" Hz\n";
 }
 // Short files and EOF must still be drained; no decoder source left pending.
 std::cout<<"Actual FillDeck integration: six duration/pitch tests passed\n";
}
