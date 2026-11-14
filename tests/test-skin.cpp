#include <windows.h>
#include <atltypes.h>
#include "../sdk/foobar2000/foo_dj/dj_skin.h"
#include <cassert>
#include <iostream>
#include <vector>
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"user32.lib")
using namespace ATL;
// Layout extracted from the actual component by the test runner.
#include "../build/tests/skin-layout.inc"
int wmain(int argc,wchar_t** argv) {
    for(auto dims : {SIZE{784,614},SIZE{900,650},SIZE{1280,720},SIZE{1920,1080}}) {
        CRect client(0,0,dims.cx,dims.cy);
        assert(FaderStrip(client).Height()<=52);
        assert(PlayBothRect(client).right<FaderLeft(client));
        assert(FaderRight(client)<LanguageRect(client,false).left);
        assert(LanguageRect(client,true).IsRectEmpty());
        for(int deck=0;deck<2;++deck) {
            auto L=MakeLayout(deck,client);
            assert(L.wave.Width()>0 && L.wave.Height()>0);
            assert(L.queue[0].bottom<=L.queue[1].top);
            assert(L.queue[1].bottom<L.wave.top);
            assert(L.queue[0].right<L.queueOpen.left);
            assert(L.queueOpen.bottom<L.wave.top);
            assert(L.analyse[0].bottom<L.queue[0].top);
            assert(L.release.right<L.automation.left);
            assert(L.automation.right<=L.deck.right);
            for(auto r : {L.btn[0],L.btn[1],L.matchKey,L.pitch[1],L.range,L.key,L.beat,L.sync,L.analyse[0]}) {
                assert(r.Width()>0 && r.Height()>0);
                assert(r.left>=L.deck.left && r.right<=L.deck.right);
            }
            assert(L.analyse[0].Height()>=40 && L.analyse[0].Width()<=64);
            assert(L.btn[0].Height()>=40);
            std::vector<CRect> row={L.gridControl[0],L.gridControl[1],L.gridControl[2],L.gridControl[3],L.pitch[0],L.pitch[1],L.pitch[2],L.matchKey,L.key};
            for(size_t n=1;n<row.size();++n) assert(row[n-1].right<=row[n].left);
            assert(L.wave.bottom<L.gridControl[0].top);
            assert(L.analyse[2].right<L.release.left);
            assert(L.release.right<=L.deck.right);
        }
    }
    HDC dc=CreateCompatibleDC(nullptr);
    BITMAPINFO bi={};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=1100;bi.bmiHeader.biHeight=-740;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
    void* pixels=nullptr;HBITMAP bitmap=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&pixels,nullptr,0);
    assert(bitmap && pixels);auto old=SelectObject(dc,bitmap);
    RECT all={0,0,1100,740};HBRUSH bg=CreateSolidBrush(RGB(16,19,25));FillRect(dc,&all,bg);DeleteObject(bg);
    CRect client(all);
    // Hover and pressed states must be visibly distinct from the normal black button.
    RECT stateRect={0,0,64,44};
    dj_skin::surface(dc,stateRect,RGB(0,194,211));GdiFlush();
    std::vector<unsigned char> normal((unsigned char*)pixels,(unsigned char*)pixels+1100*44*4);
    dj_skin::surface(dc,stateRect,RGB(0,194,211),false,true);GdiFlush();
    std::vector<unsigned char> hover((unsigned char*)pixels,(unsigned char*)pixels+1100*44*4);
    dj_skin::surface(dc,stateRect,RGB(0,194,211),false,true,true);GdiFlush();
    std::vector<unsigned char> pressed((unsigned char*)pixels,(unsigned char*)pixels+1100*44*4);
    assert(normal!=hover && hover!=pressed && normal!=pressed);
    for(int deck=0;deck<2;++deck) {
        auto L=MakeLayout(deck,client);auto accent=deck?RGB(245,143,27):RGB(0,194,211);
        dj_skin::surface(dc,L.deck,accent,true);
        RECT metadata={L.deck.left+8,L.deck.top+7,L.deck.right-8,L.deck.top+117};dj_skin::surface(dc,metadata,RGB(50,60,68));
        for(int k=0;k<3;++k) {dj_skin::surface(dc,L.analyse[k],accent);dj_skin::icon(dc,L.analyse[k],k==0?dj_skin::Icon::pulse:k==1?dj_skin::Icon::keySearch:dj_skin::Icon::refresh);}
        dj_skin::surface(dc,L.release,accent);dj_skin::icon(dc,L.release,dj_skin::Icon::release);
        for(int k=0;k<4;++k) {dj_skin::surface(dc,L.gridControl[k],accent);dj_skin::icon(dc,L.gridControl[k],k==0?dj_skin::Icon::left:k==1?dj_skin::Icon::anchor:k==2?dj_skin::Icon::right:dj_skin::Icon::automatic);}
        for(auto r:{L.pitch[0],L.pitch[1],L.pitch[2],L.matchKey,L.key,L.beat,L.sync,L.range,L.btn[0],L.btn[1]})dj_skin::surface(dc,r,accent);
        dj_skin::icon(dc,L.matchKey,dj_skin::Icon::keyMatch);dj_skin::icon(dc,L.key,dj_skin::Icon::lock,accent);
        dj_skin::icon(dc,L.beat,dj_skin::Icon::beat);dj_skin::icon(dc,L.sync,dj_skin::Icon::bpm);
        dj_skin::icon(dc,L.btn[0],dj_skin::Icon::play);dj_skin::icon(dc,L.btn[1],dj_skin::Icon::start);
        HBRUSH black=CreateSolidBrush(RGB(6,9,13));FillRect(dc,&L.wave,black);DeleteObject(black);
    }
    if(argc>1) {
        dj_skin::ready();Gdiplus::Bitmap image(bitmap,nullptr);
        const CLSID png={0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0,0,0xf8,0x1e,0xf3,0x2e}};
        assert(image.Save(argv[1],&png,nullptr)==Gdiplus::Ok);
    }
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
    std::cout<<"Skin rendering and control geometry passed; no host/window capture.\n";
}
