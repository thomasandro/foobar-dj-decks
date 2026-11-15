#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <iostream>
#include <cstring>
#include "../sdk/foobar2000/foo_dj/dj_skin.h"
#include "hover_skin_baseline.h"
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
int main(){
 HDC dc=CreateCompatibleDC(nullptr);BITMAPINFO bi={};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=1000;bi.bmiHeader.biHeight=-700;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;void* pixels=nullptr;auto image=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&pixels,nullptr,0);auto old=SelectObject(dc,image);IntersectClipRect(dc,10,10,80,42);
 dj_skin::ready();dj_skin_before::ready();LARGE_INTEGER frequency;QueryPerformanceFrequency(&frequency);
 auto run=[&](bool baseline){std::memset(pixels,0,1000*700*4);LARGE_INTEGER a,b;QueryPerformanceCounter(&a);for(int frame=0;frame<30;++frame)for(int n=0;n<120;++n){RECT r={10+(n%10)*90,10+(n/10)*48,80+(n%10)*90,42+(n/10)*48};if(baseline){dj_skin_before::surface(dc,r,RGB(0,194,211));dj_skin_before::icon(dc,r,dj_skin_before::Icon::play);}else{dj_skin::surface(dc,r,RGB(0,194,211));dj_skin::icon(dc,r,dj_skin::Icon::play);}}GdiFlush();QueryPerformanceCounter(&b);return double(b.QuadPart-a.QuadPart)*1000/frequency.QuadPart;};
 auto before=run(true);DWORD reference[70*32];int i=0;for(int y=10;y<42;++y)for(int x=10;x<80;++x)reference[i++]=((DWORD*)pixels)[y*1000+x]&0xffffff;
 auto after=run(false);i=0;for(int y=10;y<42;++y)for(int x=10;x<80;++x)assert(reference[i++]==(((DWORD*)pixels)[y*1000+x]&0xffffff));
 std::cout<<"Clipped hover rendering (30 frames/120 controls): baseline "<<before<<" ms, optimized "<<after<<" ms; visible pixels identical. Offscreen drawing only, no window capture.\n";
 SelectObject(dc,old);DeleteObject(image);DeleteDC(dc);
}
