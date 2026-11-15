#pragma once
#include <windows.h>
#include <functional>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdio>
namespace dj_queue_format {
inline const char* presets[]={"%artist% - %title% '('%length%')'","%title% - %artist% '('%length%')'","%album% | %tracknumber%. %artist% - %title% '('%length%')'","%artist% - %title%"};
inline std::string duration(double seconds){if(!std::isfinite(seconds)||seconds<=0||seconds>1e12)return "?";auto n=(unsigned long long)(seconds+.5);char out[80];if(n>=3600)std::snprintf(out,sizeof(out),"%llu:%02llu:%02llu",n/3600,n/60%60,n%60);else std::snprintf(out,sizeof(out),"%llu:%02llu",n/60,n%60);return out;}
inline std::wstring singleLine(std::wstring s){for(auto& c:s)if(c==L'\r'||c==L'\n'||c==L'\t')c=L' ';return s;}
struct Editor {
    bool en;std::wstring value;std::function<bool(const std::wstring&,std::wstring&)> validate;HFONT font=nullptr;
    static INT_PTR CALLBACK proc(HWND w,UINT msg,WPARAM wp,LPARAM lp){
        auto self=(Editor*)GetWindowLongPtrW(w,DWLP_USER);
        if(msg==WM_INITDIALOG){self=(Editor*)lp;SetWindowLongPtrW(w,DWLP_USER,(LONG_PTR)self);SetWindowTextW(w,self->en?L"Playlist display format":L"Format d'affichage des playlists");
            self->font=CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
            auto add=[&](const wchar_t* cls,const wchar_t* text,DWORD style,int id,int x,int y,int cx,int cy){auto c=CreateWindowExW(cls==std::wstring(L"EDIT")?WS_EX_CLIENTEDGE:0,cls,text,WS_CHILD|WS_VISIBLE|style,x,y,cx,cy,w,(HMENU)(INT_PTR)id,nullptr,nullptr);SendMessageW(c,WM_SETFONT,(WPARAM)self->font,TRUE);return c;};
            add(L"STATIC",self->en?L"Foobar title formatting (shared by both playlists)":L"Format foobar (commun aux deux playlists)",0,10,16,12,600,24);
            auto edit=add(L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,100,16,42,600,28);SendMessageW(edit,EM_SETLIMITTEXT,4096,0);
            add(L"STATIC",L"%artist%  %title%  %length%  %album%  %tracknumber%\n$if2(%artist%,?) - %title% '('%length%')'",0,11,16,80,600,46);
            add(L"STATIC",L"",0,101,16,136,600,70);
            add(L"BUTTON",L"OK",WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK,396,218,100,30);add(L"BUTTON",self->en?L"Cancel":L"Annuler",WS_TABSTOP,IDCANCEL,508,218,108,30);
            SetWindowPos(w,nullptr,0,0,650,300,SWP_NOMOVE|SWP_NOZORDER);SetWindowTextW(edit,self->value.c_str());SetFocus(edit);return FALSE;
        }
        if(!self)return FALSE;
        if(msg==WM_COMMAND&&((LOWORD(wp)==100&&HIWORD(wp)==EN_CHANGE)||LOWORD(wp)==IDOK)){
            int n=GetWindowTextLengthW(GetDlgItem(w,100));std::wstring value(n+1,L'\0');GetDlgItemTextW(w,100,value.data(),n+1);value.resize(n);
            std::wstring preview;bool valid=false;try{valid=!value.empty()&&self->validate(value,preview);}catch(...){preview=self->en?L"Unable to validate this expression.":L"Impossible de valider cette expression.";}SetDlgItemTextW(w,101,preview.c_str());EnableWindow(GetDlgItem(w,IDOK),valid);
            if(LOWORD(wp)==IDOK&&valid){self->value=value;EndDialog(w,IDOK);}return TRUE;
        }
        if(msg==WM_COMMAND&&LOWORD(wp)==IDCANCEL){EndDialog(w,IDCANCEL);return TRUE;}
        if(msg==WM_CLOSE){EndDialog(w,IDCANCEL);return TRUE;}
        if(msg==WM_DESTROY&&self->font){DeleteObject(self->font);self->font=nullptr;}
        return FALSE;
    }
};
inline bool edit(HWND owner,bool en,std::wstring& value,std::function<bool(const std::wstring&,std::wstring&)> validate){
    struct alignas(DWORD) Template {DLGTEMPLATE dialog;WORD menu=0,cls=0,title=0;} form={};
    form.dialog.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME;form.dialog.cx=320;form.dialog.cy=145;
    Editor state{en,value,validate};if(DialogBoxIndirectParamW(GetModuleHandleW(nullptr),&form.dialog,owner,Editor::proc,(LPARAM)&state)!=IDOK)return false;value=state.value;return true;
}
}
