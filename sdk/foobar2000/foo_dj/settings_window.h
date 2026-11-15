#pragma once
#include <commctrl.h>
#include <uxtheme.h>
#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"uxtheme.lib")
namespace dj_settings {
enum class Kind {combo,toggle,action,note};
struct Item {int id;std::wstring label;Kind kind;std::vector<std::wstring> options;int value=0;bool enabled=true;};
inline COLORREF foreground(bool enabled){return enabled?RGB(235,241,248):RGB(155,168,184);}
inline std::vector<std::wstring> tabs(bool en){return en?std::vector<std::wstring>{L"Appearance",L"Audio",L"Headphones",L"Playlists",L"Playback",L"Stems"}:std::vector<std::wstring>{L"Apparence",L"Audio",L"Casque",L"Playlists",L"Lecture",L"Stems"};}
}
// Owned, modeless and event-driven. No host polling, decoding or audio output here.
class DjSettingsWindow:public CWindowImpl<DjSettingsWindow> {
    HWND tab=nullptr,content=nullptr,closeButton=nullptr;HFONT font=nullptr;HBRUSH background=nullptr,field=nullptr;
    struct Row{HWND label=nullptr,control=nullptr;int y=0,height=0;};std::vector<Row> controls;
    std::vector<dj_settings::Item> items;int offset=0,total=0;bool rebuilding=false;
    static LRESULT CALLBACK keyProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR data){
        auto self=reinterpret_cast<DjSettingsWindow*>(data);
        if(msg==WM_SETFOCUS){for(auto& row:self->controls)if(row.control==h){RECT r;::GetClientRect(self->content,&r);if(row.y<self->offset)self->scrollBy(row.y-self->offset);else if(row.y+row.height>self->offset+r.bottom)self->scrollBy(row.y+row.height-self->offset-r.bottom);break;}}
        if(msg==WM_KEYDOWN){
            if(w==VK_ESCAPE){::PostMessage(self->m_hWnd,WM_CLOSE,0,0);return 0;}
            if(w==VK_TAB){
                if(::GetKeyState(VK_CONTROL)<0){self->select((self->selected+(::GetKeyState(VK_SHIFT)<0?5:1))%6);::SetFocus(self->tab);}
                else{auto next=::GetNextDlgTabItem(self->m_hWnd,h,::GetKeyState(VK_SHIFT)<0);if(next&&next!=h)::SetFocus(next);else ::SetFocus(self->closeButton);}
                return 0;
            }
        }
        return ::DefSubclassProc(h,msg,w,l);
    }
    static LRESULT CALLBACK contentProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR data){
        auto self=reinterpret_cast<DjSettingsWindow*>(data);
        if(msg==WM_DRAWITEM||msg==WM_MEASUREITEM||msg==WM_COMMAND||msg==WM_CTLCOLORSTATIC||msg==WM_CTLCOLORBTN||msg==WM_CTLCOLOREDIT||msg==WM_CTLCOLORLISTBOX)return ::SendMessage(self->m_hWnd,msg,w,l);
        if(msg==WM_VSCROLL){self->scroll(LOWORD(w),HIWORD(w));return 0;}
        if(msg==WM_MOUSEWHEEL){self->scrollBy(-GET_WHEEL_DELTA_WPARAM(w)/WHEEL_DELTA*self->px(90));return 0;}
        if(msg==WM_ERASEBKGND){RECT r;::GetClientRect(h,&r);::FillRect((HDC)w,&r,self->background);return 1;}
        return ::DefSubclassProc(h,msg,w,l);
    }
    HWND child(const wchar_t* cls,const wchar_t* text,DWORD style,int id,HWND parent){
        HWND h=::CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,0,0,1,1,parent,(HMENU)(INT_PTR)id,::GetModuleHandle(nullptr),nullptr);
        if(h){::SendMessage(h,WM_SETFONT,(WPARAM)font,FALSE);::SetWindowTheme(h,L"DarkMode_Explorer",nullptr);::SetWindowSubclass(h,keyProc,1,(DWORD_PTR)this);}return h;
    }
    void scrollBy(int amount){RECT r;::GetClientRect(content,&r);offset=std::clamp(offset+amount,0,(std::max)(0,total-(int)(r.bottom-r.top)));::SetScrollPos(content,SB_VERT,offset,TRUE);layoutRows();}
    void scroll(int command,int thumb){switch(command){case SB_LINEUP:scrollBy(-px(30));break;case SB_LINEDOWN:scrollBy(px(30));break;case SB_PAGEUP:scrollBy(-px(250));break;case SB_PAGEDOWN:scrollBy(px(250));break;case SB_THUMBTRACK:case SB_THUMBPOSITION:{SCROLLINFO s={sizeof(s),SIF_TRACKPOS};::GetScrollInfo(content,SB_VERT,&s);scrollBy(s.nTrackPos-offset);break;}}}
    void layoutRows(){
        if(!content)return;RECT r;::GetClientRect(content,&r);int width=r.right,split=width*43/100;
        SCROLLINFO si={sizeof(si),SIF_RANGE|SIF_PAGE|SIF_POS};si.nMin=0;si.nMax=(std::max)(0,total-1);si.nPage=r.bottom;offset=(std::min)(offset,(std::max)(0,total-(int)r.bottom));si.nPos=offset;::SetScrollInfo(content,SB_VERT,&si,TRUE);
        ::GetClientRect(content,&r);width=r.right;split=width*43/100;
        for(size_t i=0;i<controls.size();++i){const auto& item=items[i];auto& row=controls[i];int y=row.y-offset;
            if(row.label)::MoveWindow(row.label,px(12),y+px(5),split-px(20),row.height-px(8),TRUE);
            if(row.control){int x=item.kind==dj_settings::Kind::combo?split:px(12);int w=width-x-px(14);int height=item.kind==dj_settings::Kind::combo?px(230):row.height-px(8);
                ::MoveWindow(row.control,x,y,w,height,TRUE);}
        }
        ::InvalidateRect(content,nullptr,TRUE);
    }
public:
    DECLARE_WND_CLASS_EX(L"foo_dj_settings_v1",CS_DBLCLKS,(-1))
    bool english=false;int selected=0,dpi=96;COLORREF accent=RGB(0,194,211);
    std::function<std::vector<dj_settings::Item>(int)> values;
    std::function<void(int,int)> changed;
    int px(int n)const{return MulDiv(n,dpi,96);}
    void open(HWND owner,bool en,COLORREF color,int page=-1,bool visible=true){
        english=en;accent=color;if(page>=0)selected=std::clamp(page,0,5);
        if(m_hWnd){refresh();if(visible){ShowWindow(SW_RESTORE);::SetForegroundWindow(m_hWnd);}return;}
        HDC dc=::GetDC(owner);if(dc){dpi=::GetDeviceCaps(dc,LOGPIXELSX);::ReleaseDC(owner,dc);}if(dpi<96)dpi=96;
        MONITORINFO mi={sizeof(mi)};::GetMonitorInfo(::MonitorFromWindow(owner,MONITOR_DEFAULTTONEAREST),&mi);RECT o;::GetWindowRect(owner,&o);
        int w=(std::min)(px(760),(int)(mi.rcWork.right-mi.rcWork.left)),h=(std::min)(px(650),(int)(mi.rcWork.bottom-mi.rcWork.top));
        int x=std::clamp((int)o.left+px(24),(int)mi.rcWork.left,(int)mi.rcWork.right-w),y=std::clamp((int)o.top+px(24),(int)mi.rcWork.top,(int)mi.rcWork.bottom-h);
        CRect position(x,y,x+w,y+h);Create(owner,position,en?L"DJ Decks — Settings":L"DJ Decks — Réglages",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,WS_EX_TOOLWINDOW|WS_EX_CONTROLPARENT);
        if(m_hWnd&&visible){ShowWindow(SW_SHOW);::SetForegroundWindow(m_hWnd);}
    }
    void close(){if(m_hWnd)DestroyWindow();}
    void select(int page){selected=std::clamp(page,0,5);offset=0;refresh();}
    HWND control(int id)const{return content?::GetDlgItem(content,id):nullptr;}
    void refresh(){
        if(!m_hWnd||!tab||rebuilding)return;rebuilding=true;
        auto names=dj_settings::tabs(english);::SetWindowTextW(m_hWnd,english?L"DJ Decks — Settings":L"DJ Decks — Réglages");::SetWindowTextW(closeButton,english?L"Close":L"Fermer");
        TabCtrl_DeleteAllItems(tab);for(auto& name:names){TCITEMW item={TCIF_TEXT};item.pszText=name.data();TabCtrl_InsertItem(tab,TabCtrl_GetItemCount(tab),&item);}TabCtrl_SetCurSel(tab,selected);
        HWND focus=::GetFocus();int focusID=focus?::GetDlgCtrlID(focus):0;
        for(auto& row:controls){if(row.label)::DestroyWindow(row.label);if(row.control)::DestroyWindow(row.control);}controls.clear();
        items=values?values(selected):std::vector<dj_settings::Item>{};int y=px(10);
        for(auto& item:items){Row row;row.y=y;row.height=px(item.kind==dj_settings::Kind::note?72:46);
            if(item.kind==dj_settings::Kind::combo){row.label=child(L"STATIC",item.label.c_str(),SS_LEFT,0,content);row.control=child(L"COMBOBOX",L"",CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VSCROLL|WS_TABSTOP,item.id,content);
                for(auto& label:item.options)::SendMessageW(row.control,CB_ADDSTRING,0,(LPARAM)label.c_str());::SendMessage(row.control,CB_SETCURSEL,item.value,0);::SendMessage(row.control,CB_SETMINVISIBLE,8,0);
            }else if(item.kind==dj_settings::Kind::note)row.control=child(L"STATIC",item.label.c_str(),SS_LEFT,0,content);
            else{row.control=child(L"BUTTON",item.label.c_str(),WS_TABSTOP|BS_OWNERDRAW,item.id,content);}
            if(row.control)::EnableWindow(row.control,item.enabled);controls.push_back(row);y+=row.height;
        }
        total=y;rebuilding=false;layout();if(auto h=control(focusID))::SetFocus(h);::InvalidateRect(m_hWnd,nullptr,TRUE);
    }
    void layout(){if(!m_hWnd)return;CRect r;GetClientRect(&r);::MoveWindow(tab,px(12),px(46),r.Width()-px(24),px(36),TRUE);::MoveWindow(content,px(12),px(88),r.Width()-px(24),(std::max)(px(40),r.Height()-px(144)),TRUE);::MoveWindow(closeButton,(std::max)(px(12),r.Width()-px(122)),r.Height()-px(44),px(108),px(30),TRUE);layoutRows();}
    BEGIN_MSG_MAP_EX(DjSettingsWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnErase)
        MESSAGE_HANDLER(WM_NOTIFY,OnNotify)
        MESSAGE_HANDLER(WM_COMMAND,OnCommand)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnColor)
        MESSAGE_HANDLER(WM_CTLCOLORBTN,OnColor)
        MESSAGE_HANDLER(WM_CTLCOLOREDIT,OnColor)
        MESSAGE_HANDLER(WM_CTLCOLORLISTBOX,OnColor)
        MESSAGE_HANDLER(WM_DRAWITEM,OnDraw)
        MESSAGE_HANDLER(WM_GETMINMAXINFO,OnMinMax)
    END_MSG_MAP()
    int OnCreate(LPCREATESTRUCT){
        INITCOMMONCONTROLSEX init={sizeof(init),ICC_TAB_CLASSES|ICC_STANDARD_CLASSES};::InitCommonControlsEx(&init);
        font=::CreateFontW(-px(14),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");background=::CreateSolidBrush(RGB(17,21,27));field=::CreateSolidBrush(RGB(29,35,44));
        tab=child(WC_TABCONTROLW,L"",WS_TABSTOP|TCS_OWNERDRAWFIXED|TCS_FIXEDWIDTH,10,m_hWnd);TabCtrl_SetItemSize(tab,px(96),px(30));
        content=::CreateWindowExW(WS_EX_CONTROLPARENT,L"STATIC",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_CLIPCHILDREN,0,0,1,1,m_hWnd,nullptr,::GetModuleHandle(nullptr),nullptr);::SetWindowSubclass(content,contentProc,1,(DWORD_PTR)this);
        closeButton=child(L"BUTTON",L"",WS_TABSTOP|BS_OWNERDRAW,1,m_hWnd);
        using Dark=HRESULT(WINAPI*)(HWND,DWORD,LPCVOID,DWORD);HMODULE dwm=::LoadLibraryW(L"dwmapi.dll");if(dwm){auto fn=(Dark)::GetProcAddress(dwm,"DwmSetWindowAttribute");BOOL dark=TRUE;if(fn)fn(m_hWnd,20,&dark,sizeof(dark));::FreeLibrary(dwm);}
        refresh();return 0;
    }
    void OnDestroy(){controls.clear();items.clear();tab=content=closeButton=nullptr;if(font)::DeleteObject(font);font=nullptr;if(background)::DeleteObject(background);background=nullptr;if(field)::DeleteObject(field);field=nullptr;}
    void OnClose(){close();}
    void OnSize(UINT,CSize){layout();}
    BOOL OnErase(CDCHandle){return TRUE;}
    void OnPaint(CDCHandle){CPaintDC dc(m_hWnd);CRect r;GetClientRect(&r);dc.FillRect(r,background);auto old=dc.SelectFont(font);dc.SetBkMode(TRANSPARENT);dc.SetTextColor(accent);CRect title(px(18),px(10),r.right-px(16),px(40));dc.DrawText(english?L"DJ Decks settings":L"Réglages DJ Decks",-1,title,DT_SINGLELINE|DT_VCENTER);dc.SetTextColor(RGB(175,188,200));CRect hint(px(18),r.bottom-px(46),r.right-px(136),r.bottom-px(10));dc.DrawText(english?L"Changes apply immediately.":L"Modifications appliquées immédiatement.",-1,hint,DT_WORDBREAK);dc.SelectFont(old);}
    LRESULT OnNotify(UINT,WPARAM,LPARAM l,BOOL& handled){auto n=(NMHDR*)l;if(n->hwndFrom==tab&&n->code==TCN_SELCHANGE){selected=TabCtrl_GetCurSel(tab);offset=0;refresh();return 0;}handled=FALSE;return 0;}
    LRESULT OnCommand(UINT,WPARAM w,LPARAM l,BOOL&){int id=LOWORD(w),code=HIWORD(w);if(id==1){OnClose();return 0;}if(rebuilding)return 0;
        for(const auto& item:items)if(item.id==id&&item.enabled){bool combo=item.kind==dj_settings::Kind::combo;if((combo&&code!=CBN_SELCHANGE)||(!combo&&code!=BN_CLICKED)||item.kind==dj_settings::Kind::note)return 0;
            int value=combo?(int)::SendMessage((HWND)l,CB_GETCURSEL,0,0):item.kind==dj_settings::Kind::toggle?(item.value?0:1):0;
            auto callback=changed;if(callback)callback(id,value);if(m_hWnd)refresh();return 0;
        }return 0;
    }
    LRESULT OnColor(UINT msg,WPARAM w,LPARAM,BOOL&){HDC dc=(HDC)w;::SetTextColor(dc,RGB(224,232,240));::SetBkColor(dc,msg==WM_CTLCOLOREDIT||msg==WM_CTLCOLORLISTBOX?RGB(29,35,44):RGB(17,21,27));return (LRESULT)(msg==WM_CTLCOLOREDIT||msg==WM_CTLCOLORLISTBOX?field:background);}
    LRESULT OnDraw(UINT,WPARAM,LPARAM l,BOOL& handled){
        auto d=(DRAWITEMSTRUCT*)l;
        if(d->CtlType==ODT_BUTTON){
            const dj_settings::Item* entry=nullptr;for(const auto& item:items)if(item.id==(int)d->CtlID){entry=&item;break;}
            bool close=d->hwndItem==closeButton;if(!entry&&!close){handled=FALSE;return 0;}
            bool enabled=close||entry->enabled,toggle=entry&&entry->kind==dj_settings::Kind::toggle,checked=entry&&entry->value!=0;
            RECT area=d->rcItem;HDC dc=d->hDC;auto brush=::CreateSolidBrush(toggle?RGB(17,21,27):(d->itemState&ODS_SELECTED)?RGB(12,16,22):RGB(29,35,44));::FillRect(dc,&area,brush);::DeleteObject(brush);
            auto oldFont=::SelectObject(dc,font);::SetBkMode(dc,TRANSPARENT);::SetTextColor(dc,dj_settings::foreground(enabled));RECT text=area;
            if(toggle){int side=px(17),y=area.top+((area.bottom-area.top)-side)/2;RECT box={area.left+px(3),y,area.left+px(3)+side,y+side};
                auto border=::CreateSolidBrush(enabled?accent:RGB(103,115,130));::FrameRect(dc,&box,border);::DeleteObject(border);
                if(checked){auto pen=::CreatePen(PS_SOLID,px(2),enabled?accent:RGB(155,168,184));auto old=::SelectObject(dc,pen);::MoveToEx(dc,box.left+px(3),box.top+px(8),nullptr);::LineTo(dc,box.left+px(7),box.top+px(12));::LineTo(dc,box.left+px(14),box.top+px(4));::SelectObject(dc,old);::DeleteObject(pen);}
                text.left+=px(28);
            }else{auto border=::CreateSolidBrush((d->itemState&ODS_FOCUS)?accent:RGB(78,94,111));::FrameRect(dc,&area,border);::DeleteObject(border);text.left+=px(8);text.right-=px(8);}
            const wchar_t* label=close?(english?L"Close":L"Fermer"):entry->label.c_str();::DrawTextW(dc,label,-1,&text,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|(toggle?DT_LEFT:DT_CENTER));
            if(d->itemState&ODS_FOCUS){RECT focus=area;::InflateRect(&focus,-3,-3);::DrawFocusRect(dc,&focus);}::SelectObject(dc,oldFont);return TRUE;
        }
        if(d->hwndItem!=tab){handled=FALSE;return 0;}bool active=(int)d->itemID==selected;::FillRect(d->hDC,&d->rcItem,active?field:background);::SetBkMode(d->hDC,TRANSPARENT);::SetTextColor(d->hDC,active?accent:RGB(190,201,214));auto old=::SelectObject(d->hDC,font);auto names=dj_settings::tabs(english);if(d->itemID<names.size())::DrawTextW(d->hDC,names[d->itemID].c_str(),-1,&d->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);if(d->itemState&ODS_FOCUS)::DrawFocusRect(d->hDC,&d->rcItem);::SelectObject(d->hDC,old);return TRUE;
    }
    LRESULT OnMinMax(UINT,WPARAM,LPARAM l,BOOL&){auto info=(MINMAXINFO*)l;info->ptMinTrackSize={px(640),px(400)};return 0;}
};
