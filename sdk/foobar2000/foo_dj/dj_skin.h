#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#pragma comment(lib,"gdiplus.lib")
namespace dj_skin {
struct Runtime {
    ULONG_PTR token=0;
    Runtime() { Gdiplus::GdiplusStartupInput input; Gdiplus::GdiplusStartup(&token,&input,nullptr); }
    ~Runtime() { if(token) Gdiplus::GdiplusShutdown(token); }
};
inline void ready() { static Runtime runtime; }
inline Gdiplus::Color color(COLORREF c,int alpha=255) { return Gdiplus::Color((BYTE)alpha,GetRValue(c),GetGValue(c),GetBValue(c)); }
inline void rounded(Gdiplus::GraphicsPath& p,Gdiplus::RectF r,float radius) {
    const float d=(std::min)(radius*2,(std::min)(r.Width,r.Height));
    p.AddArc(r.X,r.Y,d,d,180,90);p.AddArc(r.GetRight()-d,r.Y,d,d,270,90);
    p.AddArc(r.GetRight()-d,r.GetBottom()-d,d,d,0,90);p.AddArc(r.X,r.GetBottom()-d,d,d,90,90);p.CloseFigure();
}
inline void surface(HDC dc,const RECT& rect,COLORREF accent,bool casing=false,bool active=false) {
    if(rect.right-rect.left<2 || rect.bottom-rect.top<2) return;
    ready(); Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::RectF r((float)rect.left+2,(float)rect.top+1,(float)(rect.right-rect.left)-5,(float)(rect.bottom-rect.top)-5);
    Gdiplus::GraphicsPath shadow;auto sr=r;sr.Y+=3;rounded(shadow,sr,casing?19.f:9.f);
    Gdiplus::SolidBrush shade(Gdiplus::Color(170,0,0,0));g.FillPath(&shade,&shadow);
    Gdiplus::GraphicsPath path;rounded(path,r,casing?19.f:9.f);
    auto top=casing?color(accent):Gdiplus::Color(255,45,48,52);
    auto bottom=casing?Gdiplus::Color(255,(BYTE)(GetRValue(accent)*.65),(BYTE)(GetGValue(accent)*.65),(BYTE)(GetBValue(accent)*.65)):Gdiplus::Color(255,5,7,10);
    Gdiplus::LinearGradientBrush gradient(Gdiplus::PointF(r.X,r.Y),Gdiplus::PointF(r.X,r.GetBottom()),top,bottom);
    g.FillPath(&gradient,&path);
    // A raised moulded rim and broad curved reflection, rather than a flat stripe.
    Gdiplus::Pen rim(casing?color(accent):Gdiplus::Color(255,83,89,96),2.4f);g.DrawPath(&rim,&path);
    Gdiplus::Pen outline(casing?color(accent):color(active?accent:RGB(80,87,96)),active?1.8f:1.f);g.DrawPath(&outline,&path);
    auto saved=g.Save();g.SetClip(&path);
    Gdiplus::RectF gloss(r.X+3,r.Y+2,r.Width-6,(std::min)(casing?48.f:25.f,r.Height*.48f));
    Gdiplus::LinearGradientBrush shine(Gdiplus::PointF(gloss.X,gloss.Y),Gdiplus::PointF(gloss.X,gloss.GetBottom()),Gdiplus::Color(casing?125:80,255,255,255),Gdiplus::Color(0,255,255,255));
    Gdiplus::GraphicsPath reflection;
    reflection.StartFigure();reflection.AddLine(gloss.X,gloss.Y,gloss.GetRight(),gloss.Y);
    reflection.AddBezier(gloss.GetRight(),gloss.Y,gloss.GetRight()-4,gloss.GetBottom(),gloss.X+gloss.Width*.28f,gloss.Y+gloss.Height*.55f,gloss.X,gloss.GetBottom());
    reflection.CloseFigure();g.FillPath(&shine,&reflection);
    Gdiplus::Pen lower(casing?color(accent,160):Gdiplus::Color(95,170,182,195),1.4f);
    g.DrawArc(&lower,r.X+4,r.GetBottom()-14,24.f,10.f,25.f,135.f);
    g.Restore(saved);
    Gdiplus::Pen highlight(Gdiplus::Color(casing?180:105,255,255,255),1.f);
    g.DrawLine(&highlight,r.X+8,r.Y+3,r.GetRight()-8,r.Y+3);
}
enum class Icon { play,pause,start,pulse,keySearch,refresh,left,right,anchor,automatic,lock,unlock,beat,bpm,keyMatch,both,swap,speaker };
inline void icon(HDC dc,const RECT& rect,Icon kind,COLORREF ink=RGB(239,243,247)) {
    const int width=rect.right-rect.left,height=rect.bottom-rect.top;
    if(width<8 || height<8) return;
    ready(); Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    const float size=(float)(std::min)(36,(std::min)(width-8,height-8));
    g.TranslateTransform(rect.left+(width-size)/2,rect.top+(height-size)/2);g.ScaleTransform(size/24,size/24);
    Gdiplus::Pen pen(color(ink),1.8f);pen.SetStartCap(Gdiplus::LineCapRound);pen.SetEndCap(Gdiplus::LineCapRound);
    Gdiplus::SolidBrush brush(color(ink));
    auto line=[&](float a,float b,float c,float d){g.DrawLine(&pen,a,b,c,d);};
    auto triangle=[&](float x,float y,float w,float h){Gdiplus::PointF pts[]={{x,y},{x+w,y+h/2},{x,y+h}};g.FillPolygon(&brush,pts,3);};
    auto note=[&](){line(10,5,10,16);line(10,5,17,3);line(17,3,17,14);g.FillEllipse(&brush,5.f,14.f,5.f,4.f);g.FillEllipse(&brush,12.f,12.f,5.f,4.f);};
    switch(kind) {
    case Icon::play: triangle(6,4,13,16);break;
    case Icon::pause:g.FillRectangle(&brush,6.f,4.f,4.f,16.f);g.FillRectangle(&brush,14.f,4.f,4.f,16.f);break;
    case Icon::start:line(5,4,5,20);{Gdiplus::PointF p[]={{18,4},{7,12},{18,20}};g.FillPolygon(&brush,p,3);}break;
    case Icon::both:triangle(2,5,9,14);triangle(13,5,9,14);break;
    case Icon::swap:line(3,8,20,8);line(16,4,20,8);line(16,12,20,8);line(21,16,4,16);line(8,12,4,16);line(8,20,4,16);break;
    case Icon::pulse:case Icon::keySearch:
        g.DrawEllipse(&pen,2.f,2.f,14.f,14.f);line(15,15,22,22);
        if(kind==Icon::pulse){line(4,10,7,10);line(7,10,9,5);line(9,5,11,13);line(11,13,13,9);}
        else {line(10,5,10,11);line(10,5,13,4);g.FillEllipse(&brush,6.f,10.f,4.f,3.f);}break;
    case Icon::refresh:g.DrawArc(&pen,4.f,4.f,16.f,16.f,35.f,290.f);line(19,4,19,10);line(19,10,13,10);break;
    case Icon::left:case Icon::right:{const bool left=kind==Icon::left;line(5,12,19,12);line(left?10:14,6,left?4:20,12);line(left?10:14,18,left?4:20,12);line(left?20:4,5,left?20:4,19);}break;
    case Icon::anchor:g.DrawEllipse(&pen,9.f,2.f,6.f,6.f);line(12,8,12,21);line(7,12,17,12);g.DrawArc(&pen,4.f,7.f,16.f,14.f,0.f,180.f);break;
    case Icon::automatic:g.DrawArc(&pen,2.f,2.f,20.f,20.f,30.f,290.f);line(21,3,21,9);line(21,9,16,9);{Gdiplus::FontFamily family(L"Segoe UI");Gdiplus::Font font(&family,11,Gdiplus::FontStyleBold,Gdiplus::UnitPixel);g.DrawString(L"A",1,&font,Gdiplus::PointF(7,5),&brush);}break;
    case Icon::lock:case Icon::unlock:g.DrawRectangle(&pen,5.f,10.f,14.f,11.f);g.DrawArc(&pen,8.f,2.f,8.f,12.f,kind==Icon::lock?180.f:215.f,kind==Icon::lock?180.f:140.f);g.FillEllipse(&brush,11.f,14.f,2.f,3.f);break;
    case Icon::keyMatch:note();line(13,21,23,21);line(20,18,23,21);line(20,24,23,21);break;
    case Icon::beat:line(4,4,4,20);line(9,7,9,17);line(14,4,14,20);line(19,7,19,17);line(2,12,22,12);break;
    case Icon::bpm:g.DrawEllipse(&pen,5.f,5.f,14.f,14.f);line(12,1,12,7);line(12,17,12,23);line(1,12,7,12);line(17,12,23,12);break;
    case Icon::speaker:{Gdiplus::PointF pts[]={{3,9},{7,9},{12,5},{12,19},{7,15},{3,15}};g.FillPolygon(&brush,pts,6);g.DrawArc(&pen,12.f,5.f,8.f,14.f,-70.f,140.f);}break;
    }
}
}
