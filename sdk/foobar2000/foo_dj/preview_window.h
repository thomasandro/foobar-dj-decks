#pragma once
#include "preview_visual.h"
#include "dj_skin.h"
#include <functional>
// One owned, nonactivating tool window, updated by the panel's existing UI timer.
// It never decodes a file or calls an audio output.
class DjPreviewWindow:public CWindowImpl<DjPreviewWindow> {
    HFONT font=nullptr;COLORREF accent=RGB(0,194,211);bool english=false;
    std::wstring title;dj_preview::VisualState state;
    static CRect closeRect(int width){return CRect(width-34,8,width-8,34);}
    static std::wstring time(double seconds){wchar_t text[40];auto n=(int)(seconds*10);swprintf_s(text,L"%d:%02d.%d",n/600,n/10%60,n%10);return text;}
public:
    DECLARE_WND_CLASS_EX(L"foo_dj_preview_wave_v1",CS_DBLCLKS,(-1))
    std::function<void()> stopped;
    std::function<void(double)> seeked;
    static CRect plotRect(const CRect& client){return CRect(12,50,client.right-12,client.bottom-42);}
    static double positionAt(CPoint point,const CRect& client,double duration){
        auto plot=plotRect(client);if(!plot.PtInRect(point)||plot.Width()<2||!std::isfinite(duration)||duration<=0)return -1;
        return duration*(point.x-plot.left)/(plot.Width()-1);
    }
    static CRect nearPlaylist(const RECT& playlist,const RECT& work){
        int x=playlist.right+8,y=playlist.top;
        if(x+520>work.right)x=playlist.left-528;
        if(x<work.left){x=playlist.left;y=playlist.bottom+8;if(y+190>work.bottom)y=playlist.top-198;}
        x=std::clamp(x,(int)work.left,(std::max)((int)work.left,(int)work.right-520));
        y=std::clamp(y,(int)work.top,(std::max)((int)work.top,(int)work.bottom-190));return CRect(x,y,x+520,y+190);
    }
    void open(HWND owner,const std::wstring& label,bool en,COLORREF color,HWND playlist=nullptr){
        title=label;english=en;accent=color;state={};
        HWND anchor=playlist?playlist:owner;RECT r={};::GetWindowRect(anchor,&r);MONITORINFO monitor={sizeof(monitor)};
        ::GetMonitorInfo(::MonitorFromWindow(anchor,MONITOR_DEFAULTTONEAREST),&monitor);auto rect=nearPlaylist(r,monitor.rcWork);
        if(!m_hWnd){Create(owner,rect,en?L"Headphone preview":L"Préécoute casque",WS_POPUP|WS_CLIPCHILDREN,WS_EX_TOOLWINDOW);
        }
        if(m_hWnd){SetWindowPos(nullptr,rect.left,rect.top,rect.Width(),rect.Height(),SWP_NOZORDER|SWP_NOACTIVATE);ShowWindow(SW_SHOWNOACTIVATE);Invalidate(FALSE);}
    }
    void update(const dj_preview::VisualState& value){state=value;if(m_hWnd)Invalidate(FALSE);}
    void close(){if(m_hWnd)DestroyWindow();}
    BEGIN_MSG_MAP_EX(DjPreviewWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnErase)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_LBUTTONDOWN(OnClick)
        MESSAGE_HANDLER(WM_NCHITTEST,OnHit)
        MESSAGE_HANDLER(WM_SETCURSOR,OnCursor)
    END_MSG_MAP()
    int OnCreate(LPCREATESTRUCT){font=::CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");return 0;}
    void OnDestroy(){if(font)::DeleteObject(font);font=nullptr;}
    BOOL OnErase(CDCHandle){return TRUE;}
    void OnClose(){if(stopped)stopped();else close();}
    void OnClick(UINT,CPoint point){
        CRect rect;GetClientRect(&rect);if(closeRect(rect.Width()).PtInRect(point)){OnClose();return;}
        double position=positionAt(point,rect,state.trackDuration);if(position>=0&&seeked)seeked(position);
    }
    LRESULT OnHit(UINT,WPARAM,LPARAM l,BOOL&){POINT p={GET_X_LPARAM(l),GET_Y_LPARAM(l)};ScreenToClient(&p);CRect rect;GetClientRect(&rect);return p.y<42&&!closeRect(rect.Width()).PtInRect(p)?HTCAPTION:HTCLIENT;}
    LRESULT OnCursor(UINT,WPARAM,LPARAM l,BOOL& handled){
        POINT point;::GetCursorPos(&point);ScreenToClient(&point);CRect rect;GetClientRect(&rect);
        if(LOWORD(l)==HTCLIENT&&positionAt(CPoint(point),rect,state.trackDuration)>=0){::SetCursor(::LoadCursor(nullptr,IDC_HAND));return TRUE;}
        handled=FALSE;return FALSE;
    }
    void OnPaint(CDCHandle){
        CPaintDC paint(m_hWnd);CRect client;GetClientRect(&client);if(client.IsRectEmpty())return;
        CDC dc;dc.CreateCompatibleDC(paint);CBitmap bitmap;bitmap.CreateCompatibleBitmap(paint,client.Width(),client.Height());auto oldBitmap=dc.SelectBitmap(bitmap);
        dc.FillSolidRect(client,RGB(14,18,24));CBrush border;border.CreateSolidBrush(accent);dc.FrameRect(client,border);
        CRect header(4,4,client.right-4,40);dj_skin::surface(dc,header,accent);
        auto oldFont=dc.SelectFont(font);dc.SetBkMode(TRANSPARENT);dc.SetTextColor(RGB(225,235,244));
        CRect text(14,8,client.right-46,34);dc.DrawText(title.c_str(),-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
        auto close=closeRect(client.Width());dj_skin::surface(dc,close,accent);dc.DrawText(L"×",-1,&close,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        auto plot=plotRect(client);dc.FillSolidRect(plot,RGB(7,11,16));
        int mid=(plot.top+plot.bottom)/2;CPen baseline;baseline.CreatePen(PS_SOLID,1,RGB(45,56,67));auto oldPen=dc.SelectPen(baseline);dc.MoveTo(plot.left,mid);dc.LineTo(plot.right,mid);
        float maximum=.02f;for(float peak:state.peaks)maximum=(std::max)(maximum,peak);
        CPen wave;wave.CreatePen(PS_SOLID,1,accent);dc.SelectPen(wave);
        for(int x=0;x<plot.Width();++x){auto first=(size_t)x*state.bins/plot.Width(),last=(size_t)(x+1)*state.bins/plot.Width();last=(std::max)(first+1,last);float peak=0;bool known=false;
            for(size_t n=first;n<(std::min)(last,state.bins);++n){peak=(std::max)(peak,state.peaks[n]);known=known||state.known[n];}
            if(known){int height=(int)(std::sqrt(peak/maximum)*(plot.Height()/2-3));dc.MoveTo(plot.left+x,mid-height);dc.LineTo(plot.left+x,mid+height+1);}
        }
        if(state.trackDuration>0){CPen cursor;cursor.CreatePen(PS_SOLID,2,RGB(245,247,250));dc.SelectPen(cursor);int x=plot.left+(int)((plot.Width()-1)*std::clamp(state.sourcePosition/state.trackDuration,0.,1.));dc.MoveTo(x,plot.top);dc.LineTo(x,plot.bottom);dc.SelectPen(baseline);}
        dc.SelectPen(oldPen);dc.SetTextColor(RGB(159,176,193));
        auto label=state.duration>0?(time(state.sourcePosition)+L" / "+time(state.trackDuration)+(english?L"  ·  excerpt ":L"  ·  extrait ")+time(state.position)+L" / "+time(state.duration)):(english?L"Preparing preview…":L"Préparation de l’extrait…");
        CRect status(12,client.bottom-36,client.right-12,client.bottom-18);dc.DrawText(label.c_str(),-1,&status,DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
        CRect note(12,client.bottom-19,client.right-12,client.bottom-3);dc.DrawText(state.overviewDone?(english?L"Sampled whole track · click to preview · estimated position":L"Morceau entier échantillonné · clic pour écouter · position estimée"):(english?L"Overview building / partial · click to preview":L"Aperçu en cours / partiel · clic pour écouter"),-1,&note,DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
        dc.SelectFont(oldFont);paint.BitBlt(0,0,client.Width(),client.Height(),dc,0,0,SRCCOPY);dc.SelectBitmap(oldBitmap);
    }
};
