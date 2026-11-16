#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <atlbase.h>
#include <atltypes.h>
#include "../third_party/WTL/Include/atlapp.h"
#include <atlwin.h>
#include "../third_party/WTL/Include/atlcrack.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include "../sdk/foobar2000/foo_dj/bpm_provider.h"
#include <iostream>
using namespace ATL;
using namespace WTL;
CAppModule _Module;
#include "../sdk/foobar2000/foo_dj/queue_window.h"
#include "../sdk/foobar2000/foo_dj/preview_window.h"
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"comctl32.lib")
int main(){
    for(bool smart:{false,true})for(bool classic:{false,true}){assert(dj_bpm::select(0,smart,classic)==(smart?1:classic?2:-1));assert(dj_bpm::select(1,smart,classic)==(smart?1:-1));assert(dj_bpm::select(2,smart,classic)==(classic?2:-1));}

    assert(SUCCEEDED(OleInitialize(nullptr)));_Module.Init(nullptr,GetModuleHandle(nullptr));
    INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_STANDARD_CLASSES};InitCommonControlsEx(&controls);
    DjQueueWindow a,b;CRect rect(0,0,560,400);
    assert(a.Create(nullptr,rect,L"Queue test A",DjQueueWindow::windowStyle)!=nullptr);
    assert(b.Create(nullptr,rect,L"Queue test B",DjQueueWindow::windowStyle)!=nullptr);
    assert(!a.IsWindowVisible()&&!b.IsWindowVisible());
    assert((GetWindowLongPtr(a.m_hWnd,GWL_STYLE)&WS_CAPTION)==0);
    assert((GetWindowLongPtr(a.m_hWnd,GWL_STYLE)&WS_THICKFRAME)!=0);
    RECT window;a.GetWindowRect(&window);int width=window.right-window.left,height=window.bottom-window.top;
    auto hit=[&](int x,int y){return SendMessage(a.m_hWnd,WM_NCHITTEST,0,MAKELPARAM(window.left+x,window.top+y));};
    assert(hit(1,1)==HTTOPLEFT&&hit(width-2,1)==HTTOPRIGHT);
    assert(hit(1,height-2)==HTBOTTOMLEFT&&hit(width-2,height-2)==HTBOTTOMRIGHT);
    assert(hit(width/2,1)==HTTOP&&hit(width/2,height-2)==HTBOTTOM);
    assert(hit(1,height/2)==HTLEFT&&hit(width-2,height/2)==HTRIGHT);
    assert(hit(80,22)==HTCAPTION);assert(hit(width-28,22)==HTCLIENT);assert(hit(80,100)==HTCLIENT);
    RECT listRect;GetWindowRect(a.list,&listRect);assert(listRect.top>=window.top+82);
    a.refresh({L"Title — artist",L"école",L"duplicate",L"duplicate"},{1,2});b.refresh({L"Other deck"});
    assert(a.list!=b.list);assert(SendMessage(a.list,LB_GETCOUNT,0,0)==4);
    assert((a.selected()==std::vector<size_t>{1,2}));
    // Exercise the real Ctrl+A handler, separately for each hidden playlist.
    BYTE originalKeys[256],keys[256]={};assert(GetKeyboardState(originalKeys));keys[VK_CONTROL]=0x80;assert(SetKeyboardState(keys));
    SendMessage(a.list,WM_KEYDOWN,'A',0);SendMessage(a.list,WM_CHAR,1,0);
    assert((a.selected()==std::vector<size_t>{0,1,2,3})&&b.selected().empty());
    SendMessage(b.list,WM_KEYDOWN,'A',0);assert((b.selected()==std::vector<size_t>{0}));
    b.refresh({});SendMessage(b.list,WM_KEYDOWN,'A',0);assert(b.selected().empty());
    assert(SetKeyboardState(originalKeys));a.refresh({L"Title — artist",L"école",L"duplicate",L"duplicate"},{1,2});b.refresh({L"Other deck"});
    // Exercise the actual owner-draw handlers on an offscreen bitmap, not a capture.
    HDC dc=CreateCompatibleDC(nullptr);HBITMAP bitmap=CreateBitmap(320,80,1,32,nullptr);auto prior=SelectObject(dc,bitmap);
    DRAWITEMSTRUCT draw={};draw.CtlType=ODT_LISTBOX;draw.itemID=0;draw.hwndItem=a.list;draw.hDC=dc;draw.rcItem={0,0,300,28};draw.itemState=ODS_SELECTED;
    SendMessage(a.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);assert(GetPixel(dc,1,12)==RGB(0,194,211));
    a.previewActive=0;SendMessage(a.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);
    assert(GetPixel(dc,18,14)==RGB(0,194,211)); // active row renders a solid stop square
    a.previewActive=-1;
    b.deckIndex=1;draw.hwndItem=b.list;SendMessage(b.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);assert(GetPixel(dc,1,12)==RGB(245,143,27));
    draw.itemState=0;draw.hwndItem=a.list;SendMessage(a.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);assert(GetPixel(dc,1,12)==RGB(16,21,28));
    draw.CtlType=ODT_BUTTON;draw.CtlID=101;draw.hwndItem=GetDlgItem(a.m_hWnd,101);draw.rcItem={0,0,106,36};
    SendMessage(a.m_hWnd,WM_DRAWITEM,101,(LPARAM)&draw);auto normal=GetPixel(dc,50,10);
    draw.itemState=ODS_SELECTED;SendMessage(a.m_hWnd,WM_DRAWITEM,101,(LPARAM)&draw);assert(GetPixel(dc,50,10)!=normal);
    SelectObject(dc,prior);DeleteObject(bitmap);DeleteDC(dc);
    int called=-1;std::vector<size_t> selection;
    a.action=[&](int command,std::vector<size_t> s,size_t){called=command;selection=s;};
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(106,BN_CLICKED),0);assert(called==5&&selection.empty());called=-1;
    // Escape leaves the model/window alive and preserves rows, regardless of focus.
    SendMessage(a.list,WM_KEYDOWN,VK_ESCAPE,0);assert(IsWindow(a.m_hWnd)&&SendMessage(a.list,LB_GETCOUNT,0,0)==4);
    SendMessage(GetDlgItem(b.m_hWnd,106),WM_KEYDOWN,VK_ESCAPE,0);assert(IsWindow(b.m_hWnd)&&SendMessage(b.list,LB_GETCOUNT,0,0)==1);
    int previewed=-1;a.preview=[&](size_t row){previewed=(int)row;};
    RECT cueRow;SendMessage(a.list,LB_GETITEMRECT,2,(LPARAM)&cueRow);int cy=(cueRow.top+cueRow.bottom)/2;
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(12,cy));SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(12,cy));
    assert(previewed==2&&called==-1&&(a.selected()==std::vector<size_t>{1,2}));
    previewed=-1;SendMessage(a.list,WM_LBUTTONDBLCLK,MK_LBUTTON,MAKELPARAM(12,cy));SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(12,cy));
    assert(previewed==2&&called==-1);
    previewed=-1;SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(12,cy));SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(42,cy));assert(previewed==-1);
    SendMessage(a.list,WM_KEYDOWN,VK_DELETE,0);assert(called==1&&selection.size()==2);
    SendMessage(a.list,WM_KEYDOWN,VK_RETURN,0);assert(called==0);
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(100,LBN_DBLCLK),(LPARAM)a.list);assert(called==0);
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(103,BN_CLICKED),0);assert(called==2);
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(104,BN_CLICKED),0);assert(called==3);
    // Shift held before mouse-down must not let the native list change the payload.
    // Cover contiguous and noncontiguous selections and either Shift timing.
    for(const auto& wanted:std::vector<std::vector<size_t>>{{1,2},{0,2,3}}){
        for(bool shiftAtStart:{false,true}){
            a.refresh({L"one",L"two",L"three",L"four"},wanted);
            RECT selectedRow;SendMessage(a.list,LB_GETITEMRECT,wanted.back(),(LPARAM)&selectedRow);
            int sy=(selectedRow.top+selectedRow.bottom)/2;
            SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON|(shiftAtStart?MK_SHIFT:0),MAKELPARAM(42,sy));
            assert(a.selected()==wanted);
            SendMessage(a.list,WM_MOUSEMOVE,MK_LBUTTON|MK_SHIFT,MAKELPARAM(42,sy+40));
            assert(a.selected()==wanted);
            SendMessage(a.list,WM_KEYDOWN,VK_ESCAPE,0);
            assert(a.selected()==wanted);
        }
    }
    // A plain click still collapses to one row; Shift-click keeps the native
    // extended-selection behavior, measured against an unsubclassed listbox.
    HWND native=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|LBS_EXTENDEDSEL,0,0,300,200,a.m_hWnd,nullptr,GetModuleHandle(nullptr),nullptr);
    assert(native);
    for(auto label:{L"one",L"two",L"three",L"four"})SendMessage(native,LB_ADDSTRING,0,(LPARAM)label);
    a.refresh({L"one",L"two",L"three",L"four"},{1,2});
    SendMessage(native,LB_SETSEL,TRUE,1);SendMessage(native,LB_SETSEL,TRUE,2);
    SendMessage(a.list,LB_SETANCHORINDEX,0,0);SendMessage(native,LB_SETANCHORINDEX,0,0);
    RECT ar,nr;SendMessage(a.list,LB_GETITEMRECT,2,(LPARAM)&ar);SendMessage(native,LB_GETITEMRECT,2,(LPARAM)&nr);
    LPARAM ap=MAKELPARAM(42,(ar.top+ar.bottom)/2),np=MAKELPARAM(42,(nr.top+nr.bottom)/2);
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON|MK_SHIFT,ap);
    SendMessage(a.list,WM_LBUTTONUP,MK_SHIFT,ap);
    SendMessage(native,WM_LBUTTONDOWN,MK_LBUTTON|MK_SHIFT,np);
    SendMessage(native,WM_LBUTTONUP,MK_SHIFT,np);
    for(int i=0;i<4;++i)assert(SendMessage(a.list,LB_GETSEL,i,0)==SendMessage(native,LB_GETSEL,i,0));
    DestroyWindow(native);
    a.refresh({L"one",L"two",L"three",L"four"},{1,2});
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,ap);SendMessage(a.list,WM_LBUTTONUP,0,ap);
    assert((a.selected()==std::vector<size_t>{2}));
    a.refresh({L"Title — artist",L"école",L"duplicate",L"duplicate"},{1,2});
    // Native subclass keeps a multi-selection during a drag of an already selected row.
    RECT row;SendMessage(a.list,LB_GETITEMRECT,1,(LPARAM)&row);int y=(row.top+row.bottom)/2;
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(42,y));
    SendMessage(a.list,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(42,y+50));
    SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(42,y+50));
    // Hidden windows cannot be hit by WindowFromPoint; exercise the same drop
    // dispatcher with an explicit destination, without showing/capturing a host.
    assert(a.finishDrag(a.list,{10,y+50},{1,2},false));assert(called==4&&selection.size()==2);
    bool moved=false;size_t boundary=999;DjQueueWindow* source=nullptr;
    b.received=[&](DjQueueWindow* from,const std::vector<size_t>& s,size_t before,bool move){source=from;selection=s;boundary=before;moved=move;return true;};
    assert(a.finishDrag(b.list,{10,0},{1,2},false));assert(source==&a&&selection.size()==2&&!moved&&boundary==0);
    assert(a.finishDrag(b.list,{10,1000},{1,2},true));assert(moved&&boundary==1);
    assert(!a.finishDrag(nullptr,{0,0},{1,2},false));
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(42,y));SendMessage(a.list,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(42,y+50));
    called=-1;SendMessage(a.list,WM_KEYDOWN,VK_ESCAPE,0);SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(42,y+50));assert(called==-1);called=4;
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(105,BN_CLICKED),(LPARAM)GetDlgItem(a.m_hWnd,105));
    assert(a.IsWindow()&&!a.IsWindowVisible()&&SendMessage(a.list,LB_GETCOUNT,0,0)==4&&called==4);
    SendMessage(a.m_hWnd,WM_CLOSE,0,0);assert(a.IsWindow()&&!a.IsWindowVisible());
    auto oldList=a.list;a.DestroyWindow();b.DestroyWindow();assert(!a.list&&!b.list&&DjQueueWindow::containing(oldList)==nullptr);
    DjPreviewWindow preview;CRect previewRect(0,0,520,190);
    auto adjacent=DjPreviewWindow::nearPlaylist(CRect(100,100,660,500),CRect(0,0,1920,1080));assert(adjacent.left==668&&adjacent.top==100);
    adjacent=DjPreviewWindow::nearPlaylist(CRect(1300,100,1860,500),CRect(0,0,1920,1080));assert(adjacent.right==1292);
    adjacent=DjPreviewWindow::nearPlaylist(CRect(0,0,560,500),CRect(0,0,800,600));assert(adjacent.left>=0&&adjacent.right<=800&&adjacent.top>=0&&adjacent.bottom<=600);
    assert(preview.Create(nullptr,previewRect,L"Hidden preview test",WS_POPUP,WS_EX_TOOLWINDOW)!=nullptr);
    assert(!preview.IsWindowVisible());dj_preview::VisualState visual;visual.duration=8;visual.position=2;visual.known[0]=true;visual.peaks[0]=.5f;
    visual.trackDuration=100;preview.update(visual);SendMessage(preview.m_hWnd,WM_PAINT,0,0);
    double requested=-1;preview.seeked=[&](double seconds){requested=seconds;};
    CRect pc(0,0,520,190);auto plot=DjPreviewWindow::plotRect(pc);
    SendMessage(preview.m_hWnd,WM_LBUTTONDOWN,0,MAKELPARAM(plot.left,80));assert(requested==0);
    SendMessage(preview.m_hWnd,WM_LBUTTONDOWN,0,MAKELPARAM(plot.right-1,80));assert(requested==100);
    requested=-1;SendMessage(preview.m_hWnd,WM_LBUTTONDOWN,0,MAKELPARAM(1,80));assert(requested==-1);
    RECT pr;preview.GetWindowRect(&pr);assert(SendMessage(preview.m_hWnd,WM_NCHITTEST,0,MAKELPARAM(pr.left+80,pr.top+20))==HTCAPTION);
    bool stopped=false;preview.stopped=[&]{stopped=true;preview.close();};SendMessage(preview.m_hWnd,WM_CLOSE,0,0);assert(stopped&&!preview.IsWindow());
    _Module.Term();OleUninitialize();std::cout<<"Hidden queue/preview windows: independent lists, commands, drag selection, waveform window lifecycle and clean teardown passed\n";
}
