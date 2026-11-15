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
#include <iostream>
using namespace ATL;
using namespace WTL;
CAppModule _Module;
#include "../sdk/foobar2000/foo_dj/queue_window.h"
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"comctl32.lib")
int main(){
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
    // Exercise the actual owner-draw handlers on an offscreen bitmap, not a capture.
    HDC dc=CreateCompatibleDC(nullptr);HBITMAP bitmap=CreateBitmap(320,80,1,32,nullptr);auto prior=SelectObject(dc,bitmap);
    DRAWITEMSTRUCT draw={};draw.CtlType=ODT_LISTBOX;draw.itemID=0;draw.hwndItem=a.list;draw.hDC=dc;draw.rcItem={0,0,300,28};draw.itemState=ODS_SELECTED;
    SendMessage(a.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);assert(GetPixel(dc,1,12)==RGB(0,194,211));
    b.deckIndex=1;draw.hwndItem=b.list;SendMessage(b.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);assert(GetPixel(dc,1,12)==RGB(245,143,27));
    draw.itemState=0;draw.hwndItem=a.list;SendMessage(a.m_hWnd,WM_DRAWITEM,100,(LPARAM)&draw);assert(GetPixel(dc,1,12)==RGB(16,21,28));
    draw.CtlType=ODT_BUTTON;draw.CtlID=101;draw.hwndItem=GetDlgItem(a.m_hWnd,101);draw.rcItem={0,0,106,36};
    SendMessage(a.m_hWnd,WM_DRAWITEM,101,(LPARAM)&draw);auto normal=GetPixel(dc,50,10);
    draw.itemState=ODS_SELECTED;SendMessage(a.m_hWnd,WM_DRAWITEM,101,(LPARAM)&draw);assert(GetPixel(dc,50,10)!=normal);
    SelectObject(dc,prior);DeleteObject(bitmap);DeleteDC(dc);
    int called=-1;std::vector<size_t> selection;
    a.action=[&](int command,std::vector<size_t> s,size_t){called=command;selection=s;};
    SendMessage(a.list,WM_KEYDOWN,VK_DELETE,0);assert(called==1&&selection.size()==2);
    SendMessage(a.list,WM_KEYDOWN,VK_RETURN,0);assert(called==0);
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(100,LBN_DBLCLK),(LPARAM)a.list);assert(called==0);
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(103,BN_CLICKED),0);assert(called==2);
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(104,BN_CLICKED),0);assert(called==3);
    // Native subclass keeps a multi-selection during a drag of an already selected row.
    RECT row;SendMessage(a.list,LB_GETITEMRECT,1,(LPARAM)&row);int y=(row.top+row.bottom)/2;
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(10,y));
    SendMessage(a.list,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(10,y+50));
    SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(10,y+50));
    // Hidden windows cannot be hit by WindowFromPoint; exercise the same drop
    // dispatcher with an explicit destination, without showing/capturing a host.
    assert(a.finishDrag(a.list,{10,y+50},{1,2},false));assert(called==4&&selection.size()==2);
    bool moved=false;size_t boundary=999;DjQueueWindow* source=nullptr;
    b.received=[&](DjQueueWindow* from,const std::vector<size_t>& s,size_t before,bool move){source=from;selection=s;boundary=before;moved=move;return true;};
    assert(a.finishDrag(b.list,{10,0},{1,2},false));assert(source==&a&&selection.size()==2&&!moved&&boundary==0);
    assert(a.finishDrag(b.list,{10,1000},{1,2},true));assert(moved&&boundary==1);
    assert(!a.finishDrag(nullptr,{0,0},{1,2},false));
    SendMessage(a.list,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(10,y));SendMessage(a.list,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(10,y+50));
    called=-1;SendMessage(a.list,WM_KEYDOWN,VK_ESCAPE,0);SendMessage(a.list,WM_LBUTTONUP,0,MAKELPARAM(10,y+50));assert(called==-1);called=4;
    SendMessage(a.m_hWnd,WM_COMMAND,MAKEWPARAM(105,BN_CLICKED),(LPARAM)GetDlgItem(a.m_hWnd,105));
    assert(a.IsWindow()&&!a.IsWindowVisible()&&SendMessage(a.list,LB_GETCOUNT,0,0)==4&&called==4);
    SendMessage(a.m_hWnd,WM_CLOSE,0,0);assert(a.IsWindow()&&!a.IsWindowVisible());
    auto oldList=a.list;a.DestroyWindow();b.DestroyWindow();assert(!a.list&&!b.list&&DjQueueWindow::containing(oldList)==nullptr);
    _Module.Term();OleUninitialize();std::cout<<"Hidden queue windows: independent lists, commands, drag selection and clean teardown passed\n";
}
