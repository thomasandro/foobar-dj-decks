#pragma once
#include <functional>
#include <vector>
#include <algorithm>
#include <commctrl.h>
#include "dj_skin.h"

// Owned modeless window. All callbacks and list mutations run on the UI thread.
class DjQueueWindow : public CWindowImpl<DjQueueWindow> {
public:
    DECLARE_WND_CLASS_EX(L"foo_dj_queue_v1",CS_DBLCLKS,(-1))
    std::function<bool(IDataObject*)> accepts;
    std::function<void(IDataObject*,size_t)> dropped;
    std::function<void(int,std::vector<size_t>,size_t)> action;
    std::function<void(size_t)> preview;
    int previewActive=-1;
    std::function<bool(DjQueueWindow*,const std::vector<size_t>&,size_t,bool)> received;
    static inline std::vector<DjQueueWindow*> windows;
    static DjQueueWindow* containing(HWND target){for(auto w:windows)if(w->list==target)return w;return nullptr;}
    bool finishDrag(HWND target,POINT local,const std::vector<size_t>& selection,bool moving){
        auto destination=containing(target);if(!destination)return false;
        if(destination==this){if(action)action(4,selection,boundary(local));return bool(action);}
        return destination->received&&destination->received(this,selection,destination->boundary(local),moving);
    }
    HWND list=nullptr;
    bool english=false;
    int deckIndex=0;
    static constexpr DWORD windowStyle=WS_POPUP|WS_THICKFRAME|WS_SYSMENU|WS_CLIPCHILDREN;
    static CRect CloseRect(int width){return CRect(width-46,8,width-10,40);}
    std::vector<size_t> selected() const {
        int count=(int)::SendMessage(list,LB_GETSELCOUNT,0,0);if(count<=0)return {};
        std::vector<int> raw(count);::SendMessage(list,LB_GETSELITEMS,count,(LPARAM)raw.data());
        return std::vector<size_t>(raw.begin(),raw.end());
    }
    void refresh(const std::vector<std::wstring>& labels,const std::vector<size_t>& selection={}) {
        if(!list)return;
        auto top=::SendMessage(list,LB_GETTOPINDEX,0,0);::SendMessage(list,WM_SETREDRAW,FALSE,0);
        ::SendMessage(list,LB_RESETCONTENT,0,0);
        for(auto& s:labels)::SendMessageW(list,LB_ADDSTRING,0,(LPARAM)s.c_str());
        for(auto n:selection)if(n<labels.size())::SendMessage(list,LB_SETSEL,TRUE,n);
        ::SendMessage(list,LB_SETTOPINDEX,top,0);::SendMessage(list,WM_SETREDRAW,TRUE,0);::InvalidateRect(list,nullptr,TRUE);
    }
    void open(HWND owner,int deck,bool en) {
        english=en;deckIndex=deck;
        std::wstring title=en?L"DJ Decks - Playlist ":L"DJ Decks - Liste ";title+=deck?L"B":L"A";
        if(!m_hWnd){CRect r(100+deck*80,100+deck*60,660+deck*80,500+deck*60);
            Create(owner,r,title.c_str(),windowStyle,WS_EX_TOOLWINDOW);}
        SetWindowText(title.c_str());
        const wchar_t* fr[]={L"Charger",L"Supprimer",L"Monter",L"Descendre"};
        const wchar_t* eng[]={L"Load",L"Remove",L"Move up",L"Move down"};
        for(int n=0;n<4;++n)::SetWindowTextW(buttons[n],en?eng[n]:fr[n]);
        ::SetWindowTextW(buttons[5],en?L"Empty list":L"Vider la liste");
        ::SetWindowTextW(buttons[4],en?L"Close playlist":L"Fermer la playlist");
        ::SetWindowTextW(help,en?L"Drag A/B: copy. Shift: move. Double-click: load. Delete: remove. Esc: close.":L"Glisser A/B : copier. Maj : déplacer. Double-clic : charger. Suppr : retirer. Échap : fermer.");
        ShowWindow(SW_SHOWNORMAL);SetForegroundWindow(m_hWnd);::SetFocus(list);
    }
    BEGIN_MSG_MAP_EX(DjQueueWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_GETMINMAXINFO(OnMinMax)
        MSG_WM_NCHITTEST(OnNcHitTest)
        MESSAGE_HANDLER(WM_NCCALCSIZE,OnNcCalcSize)
        MESSAGE_HANDLER(WM_NCPAINT,OnNcPaint)
        MESSAGE_HANDLER(WM_NCACTIVATE,OnNcActivate)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnErase)
        MESSAGE_HANDLER(WM_DRAWITEM,OnDrawItem)
        MESSAGE_HANDLER(WM_CTLCOLORLISTBOX,OnColor)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnColor)
        MESSAGE_HANDLER(WM_KEYDOWN,OnKeyDown)
        MESSAGE_HANDLER(WM_COMMAND,OnCommand)
    END_MSG_MAP()
private:
    HWND buttons[6]={},help=nullptr;
    HBRUSH darkBrush=nullptr;
    HFONT font=nullptr;
    int hotButton=-1;
    COLORREF accent()const{return dj_skin::accent(deckIndex);}
    LRESULT OnNcCalcSize(UINT,WPARAM,LPARAM,BOOL&){return 0;}
    LRESULT OnNcPaint(UINT,WPARAM,LPARAM,BOOL&){return 0;}
    LRESULT OnNcActivate(UINT,WPARAM,LPARAM,BOOL&){return TRUE;}
    LRESULT OnNcHitTest(CPoint screen){
        CRect r;GetWindowRect(&r);CPoint p(screen.x-r.left,screen.y-r.top);
        if(p.x<0||p.y<0||p.x>=r.Width()||p.y>=r.Height())return HTNOWHERE;
        bool left=p.x<6,right=p.x>=r.Width()-6,top=p.y<6,bottom=p.y>=r.Height()-6;
        if(top)return left?HTTOPLEFT:right?HTTOPRIGHT:HTTOP;
        if(bottom)return left?HTBOTTOMLEFT:right?HTBOTTOMRIGHT:HTBOTTOM;
        if(left)return HTLEFT;if(right)return HTRIGHT;
        if(CloseRect(r.Width()).PtInRect(p))return HTCLIENT;
        return p.y<46?HTCAPTION:HTCLIENT;
    }
    static LRESULT CALLBACK ButtonProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR data){
        auto self=(DjQueueWindow*)data;
        if(msg==WM_KEYDOWN&&w==VK_ESCAPE){self->OnClose();return 0;}
        if(msg==WM_MOUSEMOVE){
            int index=::GetDlgCtrlID(h)-101;
            if(self->hotButton!=index){int old=self->hotButton;self->hotButton=index;if(old>=0)::InvalidateRect(self->buttons[old],nullptr,FALSE);::InvalidateRect(h,nullptr,FALSE);}
            TRACKMOUSEEVENT track={sizeof(track),TME_LEAVE,h,0};::TrackMouseEvent(&track);
        }
        if(msg==WM_MOUSELEAVE){self->hotButton=-1;::InvalidateRect(h,nullptr,FALSE);}
        return DefSubclassProc(h,msg,w,l);
    }
    LRESULT OnKeyDown(UINT,WPARAM w,LPARAM,BOOL& handled){if(w==VK_ESCAPE){OnClose();return 0;}handled=FALSE;return 0;}
    BOOL OnErase(CDCHandle){return TRUE;}
    void OnPaint(CDCHandle){
        CPaintDC dc(m_hWnd);CRect r;GetClientRect(&r);
        ::FillRect(dc,&r,darkBrush);dj_skin::surface(dc,r,accent(),true);
        CRect inner=r;inner.DeflateRect(6,6);::FillRect(dc,&inner,darkBrush);
        CRect header(8,6,r.right-8,44);dj_skin::surface(dc,header,accent());
        CRect title(20,8,r.right-54,40);std::wstring name=L"Playlist ";name+=deckIndex?L"B":L"A";
        auto old=::SelectObject(dc,font);::SetBkMode(dc,TRANSPARENT);::SetTextColor(dc,accent());
        ::DrawTextW(dc,name.c_str(),-1,&title,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);::SelectObject(dc,old);
    }
    LRESULT OnColor(UINT,WPARAM w,LPARAM,BOOL&){
        auto dc=(HDC)w;::SetTextColor(dc,RGB(180,195,210));::SetBkColor(dc,RGB(16,19,25));return (LRESULT)darkBrush;
    }
    LRESULT OnDrawItem(UINT,WPARAM,LPARAM value,BOOL& handled){
        auto item=(DRAWITEMSTRUCT*)value;
        if(item->CtlType==ODT_LISTBOX){
            if(item->itemID==(UINT)-1)return TRUE;
            auto dc=item->hDC;auto r=item->rcItem;bool selected=(item->itemState&ODS_SELECTED)!=0;
            COLORREF fill=selected?RGB(16+GetRValue(accent())/4,19+GetGValue(accent())/4,25+GetBValue(accent())/4):((item->itemID%2)?RGB(23,28,35):RGB(16,21,28));
            HBRUSH brush=::CreateSolidBrush(fill);::FillRect(dc,&r,brush);::DeleteObject(brush);
            if(selected){RECT bar=r;bar.right=bar.left+3;brush=::CreateSolidBrush(accent());::FillRect(dc,&bar,brush);::DeleteObject(brush);}
            int length=(int)::SendMessage(list,LB_GETTEXTLEN,item->itemID,0);if(length<0)return TRUE;
            std::wstring text(length+1,L'\0');::SendMessageW(list,LB_GETTEXT,item->itemID,(LPARAM)text.data());
            auto old=::SelectObject(dc,font);::SetBkMode(dc,TRANSPARENT);::SetTextColor(dc,selected?RGB(244,248,252):RGB(208,218,231));
            CRect cue(r.left+5,r.top+2,r.left+31,r.bottom-2);
            dj_skin::surface(dc,cue,accent(),false,previewHot==(int)item->itemID,previewPressed==(int)item->itemID||previewActive==(int)item->itemID);
            dj_skin::icon(dc,cue,previewActive==(int)item->itemID?dj_skin::Icon::stop:dj_skin::Icon::headphones,previewActive==(int)item->itemID?accent():RGB(208,218,231));
            r.left+=38;r.right-=8;::DrawTextW(dc,text.c_str(),length,&r,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
            ::SelectObject(dc,old);if(item->itemState&ODS_FOCUS)::DrawFocusRect(dc,&item->rcItem);return TRUE;
        }
        if(item->CtlType==ODT_BUTTON){
            auto dc=item->hDC;::FillRect(dc,&item->rcItem,darkBrush);
            dj_skin::surface(dc,item->rcItem,accent(),false,(int)item->CtlID-101==hotButton,(item->itemState&ODS_SELECTED)!=0);
            wchar_t label[80]={};::GetWindowTextW(item->hwndItem,label,80);auto r=item->rcItem;
            auto old=::SelectObject(dc,font);::SetBkMode(dc,TRANSPARENT);::SetTextColor(dc,RGB(239,243,247));
            if(item->itemState&ODS_SELECTED)::OffsetRect(&r,0,1);
            if(item->CtlID==105){
                HPEN pen=::CreatePen(PS_SOLID,2,RGB(239,243,247));auto prior=::SelectObject(dc,pen);
                int x=(r.left+r.right)/2,y=(r.top+r.bottom)/2;
                ::MoveToEx(dc,x-5,y-5,nullptr);::LineTo(dc,x+6,y+6);::MoveToEx(dc,x+5,y-5,nullptr);::LineTo(dc,x-6,y+6);
                ::SelectObject(dc,prior);::DeleteObject(pen);
            }else ::DrawTextW(dc,label,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX);
            ::SelectObject(dc,old);if(item->itemState&ODS_FOCUS){::InflateRect(&r,-7,-7);::DrawFocusRect(dc,&r);}return TRUE;
        }
        handled=FALSE;return 0;
    }
    CPoint start;bool dragging=false,preserveSelection=false;
    int previewHot=-1,previewPressed=-1;
    WPARAM deferredSelectionKeys=0;
    std::vector<size_t> dragSelection;
    class Target : public IDropTarget {
    public:
        DjQueueWindow* window;LONG refs=1;bool ok=false;
        Target(DjQueueWindow* w):window(w){}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id,void** out)override {
            if(!out)return E_POINTER;*out=nullptr;
            if(id==IID_IUnknown||id==IID_IDropTarget){*out=this;AddRef();return S_OK;}return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&refs);}
        ULONG STDMETHODCALLTYPE Release()override{auto n=InterlockedDecrement(&refs);if(!n)delete this;return n;}
        HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* o,DWORD,POINTL,DWORD* e)override {
            ok=window&&window->accepts&&window->accepts(o);if(e)*e=ok?DROPEFFECT_COPY:DROPEFFECT_NONE;return S_OK;
        }
        HRESULT STDMETHODCALLTYPE DragOver(DWORD,POINTL,DWORD* e)override {if(e)*e=ok?DROPEFFECT_COPY:DROPEFFECT_NONE;return S_OK;}
        HRESULT STDMETHODCALLTYPE DragLeave()override {ok=false;return S_OK;}
        HRESULT STDMETHODCALLTYPE Drop(IDataObject* o,DWORD,POINTL p,DWORD* e)override {
            bool accept=ok&&window&&window->dropped;
            if(accept){POINT q={p.x,p.y};::ScreenToClient(window->list,&q);window->dropped(o,window->boundary(q));}
            if(e)*e=accept?DROPEFFECT_COPY:DROPEFFECT_NONE;ok=false;return S_OK;
        }
    };
    Target* target=nullptr;
    size_t boundary(POINT p)const {
        int count=(int)::SendMessage(list,LB_GETCOUNT,0,0);if(count<=0)return 0;
        RECT client;::GetClientRect(list,&client);
        if(p.y<0)return (size_t)::SendMessage(list,LB_GETTOPINDEX,0,0);
        DWORD hit=(DWORD)::SendMessage(list,LB_ITEMFROMPOINT,0,MAKELPARAM(p.x,p.y));
        int index=LOWORD(hit);RECT row={};::SendMessage(list,LB_GETITEMRECT,index,(LPARAM)&row);
        return (std::min)((size_t)count,(size_t)(index+(p.y>=(row.top+row.bottom)/2)));
    }
    static LRESULT CALLBACK ListProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR data) {
        auto self=(DjQueueWindow*)data;
        if(msg==WM_MOUSEMOVE&&!(w&MK_LBUTTON)){
            DWORD hit=(DWORD)::SendMessage(h,LB_ITEMFROMPOINT,0,l);int hot=GET_X_LPARAM(l)>=5&&GET_X_LPARAM(l)<31&&!HIWORD(hit)?LOWORD(hit):-1;
            if(hot!=self->previewHot){self->previewHot=hot;::InvalidateRect(h,nullptr,FALSE);TRACKMOUSEEVENT track={sizeof(track),TME_LEAVE,h,0};::TrackMouseEvent(&track);}
        }
        if(msg==WM_MOUSELEAVE){self->previewHot=-1;::InvalidateRect(h,nullptr,FALSE);}
        if((msg==WM_LBUTTONDOWN||msg==WM_LBUTTONDBLCLK)&&GET_X_LPARAM(l)>=5&&GET_X_LPARAM(l)<31){
            DWORD hit=(DWORD)::SendMessage(h,LB_ITEMFROMPOINT,0,l);if(!HIWORD(hit)&&LOWORD(hit)<::SendMessage(h,LB_GETCOUNT,0,0)){
                self->dragging=false;self->preserveSelection=false;self->dragSelection.clear();
                self->previewPressed=LOWORD(hit);::SetFocus(h);::SetCapture(h);::InvalidateRect(h,nullptr,FALSE);return 0;}
        }
        if(msg==WM_LBUTTONUP&&self->previewPressed>=0){
            int pressed=self->previewPressed;self->previewPressed=-1;DWORD hit=(DWORD)::SendMessage(h,LB_ITEMFROMPOINT,0,l);
            if(::GetCapture()==h)::ReleaseCapture();::InvalidateRect(h,nullptr,FALSE);
            if(!HIWORD(hit)&&LOWORD(hit)==pressed&&GET_X_LPARAM(l)>=5&&GET_X_LPARAM(l)<31&&self->preview)self->preview(pressed);return 0;
        }
        if(msg==WM_CAPTURECHANGED||msg==WM_CANCELMODE||(msg==WM_KEYDOWN&&w==VK_ESCAPE))self->previewPressed=-1;
        if((msg==WM_KEYDOWN&&w==VK_ESCAPE)||msg==WM_CANCELMODE){bool gesture=self->dragging||self->preserveSelection||::GetCapture()==h;self->dragging=false;self->preserveSelection=false;self->dragSelection.clear();if(::GetCapture()==h)::ReleaseCapture();if(msg==WM_KEYDOWN&&!gesture)self->OnClose();return 0;}
        if(msg==WM_KEYDOWN && w=='A' && (::GetKeyState(VK_CONTROL)&0x8000) && !(::GetKeyState(VK_MENU)&0x8000)){
            ::SendMessage(h,LB_SETSEL,TRUE,-1);::InvalidateRect(h,nullptr,FALSE);return 0;
        }
        if(msg==WM_CHAR && w==1)return 0; // Ctrl+A: consume the translated character, without a beep.
        if(msg==WM_KEYDOWN && w==VK_DELETE){if(self->action)self->action(1,self->selected(),0);return 0;}
        if(msg==WM_KEYDOWN && w==VK_RETURN){if(self->action)self->action(0,self->selected(),0);return 0;}
        if(msg==WM_LBUTTONDOWN){
            self->start=CPoint(GET_X_LPARAM(l),GET_Y_LPARAM(l));
            DWORD hit=(DWORD)::SendMessage(h,LB_ITEMFROMPOINT,0,l);
            // Shift selects a range on a click, but means "move" on a drag.
            // Defer native selection changes on an already selected row until
            // mouse-up proves this is a click, preserving the complete payload.
            self->preserveSelection=!(w&MK_CONTROL)&&!HIWORD(hit)&&::SendMessage(h,LB_GETSEL,LOWORD(hit),0)>0;
            self->deferredSelectionKeys=w;
            LRESULT result=0;
            if(self->preserveSelection){::SetFocus(h);::SetCapture(h);}
            else result=DefSubclassProc(h,msg,w,l);
            self->dragSelection=self->selected();self->dragging=false;return result;
        }
        if(msg==WM_MOUSEMOVE && (w&MK_LBUTTON) && !self->dragSelection.empty()) {
            int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);
            if(abs(x-self->start.x)>GetSystemMetrics(SM_CXDRAG)||abs(y-self->start.y)>GetSystemMetrics(SM_CYDRAG))self->dragging=true;
            if(self->dragging){POINT screen={x,y};::ClientToScreen(h,&screen);auto target=containing(::WindowFromPoint(screen));
                ::SetCursor(::LoadCursor(nullptr,target?(target==self?IDC_SIZEALL:IDC_HAND):IDC_NO));
                if(target){POINT q=screen;::ScreenToClient(target->list,&q);RECT r;::GetClientRect(target->list,&r);auto top=::SendMessage(target->list,LB_GETTOPINDEX,0,0);
                    if(q.y<14&&top>0)::SendMessage(target->list,LB_SETTOPINDEX,top-1,0);
                    if(q.y>r.bottom-14)::SendMessage(target->list,LB_SETTOPINDEX,top+1,0);}return 0;}
        }
        if(msg==WM_LBUTTONUP && self->dragging){
            POINT screen={GET_X_LPARAM(l),GET_Y_LPARAM(l)};::ClientToScreen(h,&screen);HWND target=::WindowFromPoint(screen);POINT local=screen;if(target)::ScreenToClient(target,&local);
            auto sel=self->dragSelection;self->dragging=false;self->preserveSelection=false;self->dragSelection.clear();
            DefSubclassProc(h,msg,w,l);if(::GetCapture()==h)::ReleaseCapture();self->finishDrag(target,local,sel,(w&MK_SHIFT)!=0);return 0;
        }
        if(msg==WM_LBUTTONUP && self->preserveSelection){
            self->preserveSelection=false;self->dragSelection.clear();::ReleaseCapture();
            if(self->deferredSelectionKeys&MK_SHIFT){
                DefSubclassProc(h,WM_LBUTTONDOWN,self->deferredSelectionKeys,MAKELPARAM(self->start.x,self->start.y));
                return DefSubclassProc(h,msg,w,l);
            }
            DWORD hit=(DWORD)::SendMessage(h,LB_ITEMFROMPOINT,0,l);
            if(!HIWORD(hit)){::SendMessage(h,LB_SETSEL,FALSE,-1);::SendMessage(h,LB_SETSEL,TRUE,LOWORD(hit));}return 0;
        }
        if(msg==WM_CAPTURECHANGED){self->dragging=false;self->dragSelection.clear();}
        return DefSubclassProc(h,msg,w,l);
    }
    int OnCreate(LPCREATESTRUCT) {
        darkBrush=::CreateSolidBrush(RGB(16,19,25));
        font=::CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        list=::CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|LBS_NOTIFY|LBS_EXTENDEDSEL|LBS_NOINTEGRALHEIGHT|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS,0,0,0,0,m_hWnd,(HMENU)100,nullptr,nullptr);
        ::SendMessage(list,LB_SETITEMHEIGHT,0,28);
        ::SetWindowSubclass(list,ListProc,1,(DWORD_PTR)this);
        for(int n=0;n<4;++n){buttons[n]=::CreateWindowW(L"BUTTON",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,m_hWnd,(HMENU)(INT_PTR)(101+n),nullptr,nullptr);::SetWindowSubclass(buttons[n],ButtonProc,1,(DWORD_PTR)this);}
        buttons[4]=::CreateWindowW(L"BUTTON",english?L"Close playlist":L"Fermer la playlist",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,m_hWnd,(HMENU)105,nullptr,nullptr);
        ::SetWindowSubclass(buttons[4],ButtonProc,1,(DWORD_PTR)this);
        buttons[5]=::CreateWindowW(L"BUTTON",english?L"Empty list":L"Vider la liste",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,0,0,0,0,m_hWnd,(HMENU)106,nullptr,nullptr);
        ::SetWindowSubclass(buttons[5],ButtonProc,1,(DWORD_PTR)this);
        help=::CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE,0,0,0,0,m_hWnd,nullptr,nullptr,nullptr);
        for(auto h:{list,help,buttons[0],buttons[1],buttons[2],buttons[3],buttons[5]})::SendMessage(h,WM_SETFONT,(WPARAM)font,FALSE);
        target=new Target(this);if(FAILED(::RegisterDragDrop(list,target))){target->Release();target=nullptr;}
        windows.push_back(this);
        return 0;
    }
    void OnSize(UINT,CSize size){
        auto close=CloseRect(size.cx);::MoveWindow(buttons[4],close.left,close.top,close.Width(),close.Height(),TRUE);
        ::MoveWindow(help,8,50,(std::max)(1,(int)size.cx-16),30,TRUE);
        ::MoveWindow(list,8,82,(std::max)(1,(int)size.cx-16),(std::max)(1,(int)size.cy-130),TRUE);
        int width=((int)size.cx-16)/5;
        for(int n=0;n<5;++n)::MoveWindow(buttons[n==4?5:n],8+n*width,size.cy-43,width-4,36,TRUE);
        Invalidate(FALSE);
    }
    void OnClose(){if(list)::SendMessage(list,WM_CANCELMODE,0,0);previewPressed=-1;ShowWindow(SW_HIDE);}
    void OnMinMax(MINMAXINFO* info){info->ptMinTrackSize.x=480;info->ptMinTrackSize.y=240;}
    void OnDestroy(){windows.erase(std::remove(windows.begin(),windows.end(),this),windows.end());if(target){::RevokeDragDrop(list);target->window=nullptr;target->Release();target=nullptr;}::RemoveWindowSubclass(list,ListProc,1);for(auto h:buttons)::RemoveWindowSubclass(h,ButtonProc,1);list=nullptr;if(font){::DeleteObject(font);font=nullptr;}if(darkBrush){::DeleteObject(darkBrush);darkBrush=nullptr;}}
    LRESULT OnCommand(UINT,WPARAM w,LPARAM,BOOL&) {
        if(LOWORD(w)==100 && HIWORD(w)==LBN_DBLCLK){if(action)action(0,selected(),0);}
        else if(LOWORD(w)>=101&&LOWORD(w)<=104){if(action)action(LOWORD(w)-101,selected(),0);}
        else if(LOWORD(w)==105)OnClose();
        else if(LOWORD(w)==106){if(action)action(5,{},0);}
        return 0;
    }
};
