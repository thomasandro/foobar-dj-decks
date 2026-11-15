int main(){
    CDjPanel panel;abort_callback_impl abort;
    for(int mode=0;mode<5;++mode){auto opened=fake_preview::opened;panel.m_cueOutput.data.clear();panel.m_previewVisual.reset(1);panel.PlayPreview({{"file://memory-only",7},mode,1},abort);assert(fake_preview::opened==opened+1);
        size_t frames=panel.m_cueOutput.data.size()/2;assert(frames==(mode==0||mode==2?8000:mode==4?16000:15000));assert(fake_preview::subsong==7&&fake_preview::alive==0);
        size_t start=mode==2?46000:mode==3?42500:0;
        assert(std::abs(panel.m_cueOutput.data[0]-(start+1)*.2)<1e-6);
        for(size_t n=0;n<frames;++n)assert(panel.m_cueOutput.data[n*2]==-panel.m_cueOutput.data[n*2+1]);
        auto visual=panel.m_previewVisual.snapshot(0);assert(std::abs(visual.submitted-frames/1000.)<1e-6);assert(visual.position==visual.duration||std::abs(visual.position-visual.duration)<1e-6);
        assert(visual.trackDuration==100);
        if(mode==2||mode==3)assert(!visual.known[0]); // middle samples map to the middle of the whole track
    }
    for(int mode:{0,1,4}){panel.m_cueOutput.data.clear();panel.m_previewVisual.reset(1);panel.PlayPreview({{"file://memory-only",7},mode,1,30},abort);
        assert(panel.m_cueOutput.data.size()/2==(mode==0?8000:mode==1?15000:16000));assert(std::abs(panel.m_cueOutput.data[0]-30001*.2)<1e-6);
        auto visual=panel.m_previewVisual.snapshot(0);assert(std::abs(visual.sourcePosition-(30+visual.duration))<1e-6);}
    panel.m_cueOutput.data.clear();panel.PlayPreview({{"file://memory-only",0},0,1,98},abort);assert(panel.m_cueOutput.data.size()/2==2000);
    fake_preview::length=3;panel.m_cueOutput.data.clear();panel.PlayPreview({{"file://memory-only",0},4,1},abort);assert(panel.m_cueOutput.data.size()==6000&&fake_preview::alive==0);
    fake_preview::length=100;fake_preview::seekable=false;bool failed=false;
    try{panel.PlayPreview({{"file://memory-only",0},2,1},abort);}catch(...){failed=true;}assert(failed&&fake_preview::alive==0);fake_preview::seekable=true;
    abort.cancelled=true;failed=false;try{panel.PlayPreview({{"file://memory-only",0},0,1},abort);}catch(...){failed=true;}assert(failed&&fake_preview::alive==0);abort.cancelled=false;
    panel.m_cueOutput.fail=true;failed=false;try{panel.PlayPreview({{"file://memory-only",0},0,1},abort);}catch(...){failed=true;}assert(failed&&fake_preview::alive==0);
    panel.m_cueOutput.fail=false;panel.m_cueOutput.generation=2;panel.m_cueOutput.data.clear();panel.PlayPreview({{"file://memory-only",0},0,1},abort);assert(panel.m_cueOutput.data.empty()&&fake_preview::alive==0);
    auto before=fake_preview::opened;panel.m_previewVisual.reset(9);panel.ScanPreviewOverview({{"file://overview-memory-only",4},0,9},abort);
    auto overview=panel.m_previewVisual.snapshot(0);assert(overview.overviewDone&&overview.trackDuration==100&&overview.known[0]&&overview.known[1023]&&fake_preview::alive==0);
    assert(fake_preview::opened==before+1);before=fake_preview::opened;
    panel.ScanPreviewOverview({{"file://overview-memory-only",4},0,9},abort);assert(fake_preview::opened==before); // cache avoids reopening a decoder
    for(int n=0;n<9;++n){dj_preview::Overview entry;entry.length=n+1;panel.m_previewOverviewCache.put("bounded"+std::to_string(n),entry);}
    assert(panel.m_previewOverviewCache.get("bounded0").length==0&&panel.m_previewOverviewCache.get("bounded8").length==9);
    std::cout<<"Actual PlayPreview: five modes/sample counts, centered seeks, subsongs, fades/stereo, short tracks, cancellation, unavailable headphones, stale generations and decoder release passed (mock input/sink; no files/audio opened)\n";
}
