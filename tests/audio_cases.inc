int main(){for(bool fixed : {false,true}) for(double speed : {0.5,0.84,1.0,1.16,1.5}) for(int pitch : {-12,0,7}){
 Deck d;abort_callback abort;std::vector<float> result;
 for(int pass=0;pass<2000;++pass){FillDeck(d,1024,48000,abort,speed,fixed,pitch);size_t n=(std::min)((size_t)1024,d.pendingFrames());result.insert(result.end(),d.pending.begin()+d.pendingPos,d.pending.begin()+d.pendingPos+n*2);d.pendingPos+=n*2;if(d.pendingPos==d.pending.size()){d.pending.clear();d.pendingPos=0;}if(d.eof&&d.pendingFrames()==0)break;}
 double duration=result.size()/2.0/48000;assert(std::abs(duration-3/speed)<0.025);
 size_t first=24000,last=result.size()/2-24000;unsigned crossings=0;for(size_t n=first+1;n<last;++n)if(result[2*(n-1)]<=0&&result[2*n]>0)++crossings;
 double hz=crossings*48000.0/(last-first),expected=440*(fixed?1:speed)*std::pow(2.0,pitch/12.0);assert(std::abs(hz-expected)<2.0);
 std::cout<<"fixed="<<fixed<<" speed="<<speed<<" duration="<<duration<<" frequency="<<hz<<" Hz\n";
 }
 // A tag-analysis suspension must release the only decoder owner and all audio buffers,
 // while preserving position for explicit resume. No reader is reopened here.
 { Deck d; d.framesPlayed=45678; d.src={1,2}; d.pending={3,4};
   d.stretch=std::make_shared<soundtouch::SoundTouch>();
   std::weak_ptr<Input> reader=d.helper;
   ReleaseDeckReader(d);
   assert(reader.expired() && !d.helper && !d.playing && d.framesPlayed==45678);
   assert(d.src.empty() && d.pending.empty() && !d.stretch && d.srcPos==0 && d.pendingPos==0);
 }
 // Short files and EOF must still be drained; no decoder source left pending.
 std::cout<<"Actual FillDeck integration: 30 duration/pitch/transposition tests passed\n";
 std::wstring folder=L"build\\tests\\stem-audio";std::filesystem::create_directories(folder);std::string key(64,'e');
 picojson::object manifest;manifest["engine"]=picojson::value(std::string(dj_stems::engine));manifest["key"]=picojson::value(key);manifest["rate"]=picojson::value(44100.);manifest["channels"]=picojson::value(2.);manifest["frames"]=picojson::value(132300.);
 {std::ofstream out(folder+L"\\manifest.json");out<<picojson::value(manifest).serialize();}
 for(int k=0;k<4;++k){std::vector<float> samples;for(int n=0;n<132300;++n){float x=(float)(.1*sin(n*2*3.141592653589793*110*(k+1)/44100));samples.push_back(x);samples.push_back(x);}std::ofstream out(folder+L"\\"+dj_stems::names[k],std::ios::binary);out.write((const char*)samples.data(),samples.size()*4);}
 for(bool fixed:{false,true})for(double speed:{.5,.84,1.,1.16,1.5})for(int pitch:{-12,0,7}){
     Deck d;d.rate=44100;d.stems=std::make_shared<dj_stems::Reader>(folder,key);d.stems->mask=2;d.stems->seek(0);abort_callback abort;std::vector<float> result;
     for(int pass=0;pass<2000;++pass){FillDeck(d,1024,48000,abort,speed,fixed,pitch);size_t n=(std::min)((size_t)1024,d.pendingFrames());result.insert(result.end(),d.pending.begin()+d.pendingPos,d.pending.begin()+d.pendingPos+n*2);d.pendingPos+=n*2;if(d.pendingPos==d.pending.size()){d.pending.clear();d.pendingPos=0;}if(d.eof&&d.pendingFrames()==0)break;}
     double duration=result.size()/2./48000;assert(std::abs(duration-3/speed)<.025);size_t first=24000,last=result.size()/2-24000;unsigned crossings=0;for(size_t n=first+1;n<last;++n)if(result[2*(n-1)]<=0&&result[2*n]>0)++crossings;
     double hz=crossings*48000./(last-first),expected=220*(fixed?1:speed)*std::pow(2.,pitch/12.);assert(std::abs(hz-expected)<2.);
     assert(d.helper->pos==0); // Stem mode must not consume the original decoder.
 }
 {Deck d;d.stems=std::make_shared<dj_stems::Reader>(folder,key);std::weak_ptr<dj_stems::Reader> reader=d.stems;ReleaseDeckReader(d);assert(reader.expired()&&!d.stems&&!d.helper);}
 std::cout<<"Actual FillDeck stems: 30 additional tempo/pitch/key-lock cases, original-reader isolation and stem mapping release passed\n";
}
