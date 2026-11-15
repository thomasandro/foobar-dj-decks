#pragma once
#include "stem_cache.h"
#include <filesystem>
#include <set>
#include <future>
#include <memory>
#include <commctrl.h>
#include <chrono>
namespace dj_stems {
struct CacheReport { uint64_t bytes=0,removedBytes=0;size_t entries=0,removed=0,kept=0,errors=0; };
inline bool cacheKeyName(const std::wstring& s){return s.size()==64&&s.find_first_not_of(L"0123456789abcdef")==std::wstring::npos;}
inline bool plainDirectory(const std::wstring& p){auto a=GetFileAttributesW(p.c_str());return a!=INVALID_FILE_ATTRIBUTES&&(a&FILE_ATTRIBUTE_DIRECTORY)&&!(a&FILE_ATTRIBUTE_REPARSE_POINT);}
inline bool safeCacheRoot(){return plainDirectory(localRoot())&&plainDirectory(localRoot()+L"\\stems-cache")&&plainDirectory(cacheRoot());}
struct CacheHandle {HANDLE h=INVALID_HANDLE_VALUE;~CacheHandle(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);}CacheHandle()=default;CacheHandle(const CacheHandle&)=delete;};
// Never recurse or delete arbitrary paths. Only the five fixed files in a direct
// 64-hex cache child are eligible; jobs, lock files and unknown data are retained.
inline CacheReport inspectCache(bool clear,const std::set<std::wstring>& protectedKeys={}){
    CacheReport report;auto root=cacheRoot();
    if(GetFileAttributesW(root.c_str())==INVALID_FILE_ATTRIBUTES){if(GetLastError()!=ERROR_FILE_NOT_FOUND&&GetLastError()!=ERROR_PATH_NOT_FOUND)++report.errors;return report;}
    if(!safeCacheRoot()){++report.errors;return report;}
    try {for(const auto& entry:std::filesystem::directory_iterator(root)){
        auto name=entry.path().filename().wstring();if(!cacheKeyName(name))continue;
        auto dir=entry.path().wstring();if(!plainDirectory(dir)){++report.errors;continue;}
        ++report.entries;uint64_t bytes=0;bool valid=true;std::vector<std::wstring> paths;
        for(const auto& f:std::filesystem::directory_iterator(dir)){
            auto leaf=f.path().filename().wstring();bool known=leaf==L"manifest.json";
            for(auto n:names)known=known||leaf==n;
            auto a=GetFileAttributesW(f.path().c_str());
            if(a==INVALID_FILE_ATTRIBUTES||(a&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY))){valid=false;continue;}
            bytes+=f.file_size();if(!known||(a&FILE_ATTRIBUTE_READONLY))valid=false;paths.push_back(f.path().wstring());
        }
        report.bytes+=bytes;
        if(!clear)continue;
        if(protectedKeys.count(name)||!valid||paths.size()!=5){++report.kept;continue;}
        CacheHandle keyLock;keyLock.h=CreateFileW((root+L"\\"+name+L".lock").c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
        OVERLAPPED ov={};FILE_ATTRIBUTE_TAG_INFO tag={};
        if(keyLock.h==INVALID_HANDLE_VALUE||!GetFileInformationByHandleEx(keyLock.h,FileAttributeTagInfo,&tag,sizeof(tag))||(tag.FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)||!LockFileEx(keyLock.h,LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,0,1,0,&ov)){++report.kept;continue;}
        // Preflight EVERY file before removing any: readers that omit SHARE_DELETE
        // protect their entire entry. DELETE access also blocks new readers until done.
        std::vector<std::unique_ptr<CacheHandle>> handles;
        for(const auto& path:paths){auto h=std::make_unique<CacheHandle>();h->h=CreateFileW(path.c_str(),GENERIC_READ|DELETE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
            FILE_ATTRIBUTE_TAG_INFO t={};if(h->h==INVALID_HANDLE_VALUE||!GetFileInformationByHandleEx(h->h,FileAttributeTagInfo,&t,sizeof(t))||(t.FileAttributes&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY))){valid=false;break;}handles.push_back(std::move(h));}
        if(!valid){++report.kept;continue;}
        bool deleted=true;FILE_DISPOSITION_INFO disposition={TRUE};
        for(auto& h:handles)if(!SetFileInformationByHandle(h->h,FileDispositionInfo,&disposition,sizeof(disposition)))deleted=false;
        handles.clear();
        if(deleted&&RemoveDirectoryW(dir.c_str())){++report.removed;report.removedBytes+=bytes;}else ++report.errors;
    }}catch(const std::exception&){++report.errors;}
    return report;
}
inline std::wstring cacheSize(uint64_t bytes){wchar_t text[80];swprintf_s(text,L"%.1f MiB",double(bytes)/(1024*1024));return text;}
struct CacheDialog {
    bool en;std::set<std::wstring> protectedKeys;std::future<CacheReport> task;CacheReport report;bool running=false,clearing=false;
    void start(HWND window,bool clear){clearing=clear;running=true;SendMessageW(window,TDM_ENABLE_BUTTON,100,FALSE);SendMessageW(window,TDM_ENABLE_BUTTON,101,FALSE);SendMessageW(window,TDM_SET_ELEMENT_TEXT,TDE_CONTENT,(LPARAM)(clear?(en?L"Clearing unused cache…":L"Nettoyage du cache inutilisé…"):(en?L"Reading cache size…":L"Calcul de la taille du cache…")));auto keys=protectedKeys;task=std::async(std::launch::async,[clear,keys]{SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);auto r=inspectCache(clear,keys);if(clear){auto after=inspectCache(false);r.bytes=after.bytes;r.entries=after.entries;r.errors+=after.errors;}return r;});}
    static HRESULT CALLBACK callback(HWND window,UINT event,WPARAM w,LPARAM,LONG_PTR data){auto& d=*reinterpret_cast<CacheDialog*>(data);
        if(event==TDN_CREATED){try{d.start(window,false);}catch(...){return E_FAIL;}}
        if(event==TDN_BUTTON_CLICKED){if(d.running)return S_FALSE;if(w==100||w==101){try{d.start(window,w==100);}catch(...){return E_FAIL;}return S_FALSE;}}
        if(event==TDN_TIMER&&d.running&&d.task.wait_for(std::chrono::seconds(0))==std::future_status::ready){
            try{d.report=d.task.get();}catch(...){++d.report.errors;}
            d.running=false;auto& r=d.report;
            std::wstring text=(d.en?L"Prepared stems: ":L"Stems préparés : ")+cacheSize(r.bytes)+L"\n"+std::to_wstring(r.entries)+(d.en?L" cache entries.":L" morceaux en cache.");
            if(d.clearing)text+=L"\n"+std::to_wstring(r.removed)+(d.en?L" removed; ":L" supprimés ; ")+cacheSize(r.removedBytes)+(d.en?L" freed.\n":L" libérés.\n")+std::to_wstring(r.kept)+(d.en?L" retained (in use, protected or unknown contents).":L" conservés (utilisés, protégés ou contenu inconnu).");
            if(r.errors)text+=L"\n"+std::to_wstring(r.errors)+(d.en?L" access/deletion errors; some files were retained.":L" erreurs d'accès/suppression ; certains fichiers sont conservés.");
            text+=d.en?L"\n\nClearing preserves active/preparing stems, temporary jobs, unknown files and the AI engine/model. Source music is untouched. To remove active stems too, choose Original audio or Stop/Release on both decks, then reopen this manager.\nSize excludes temporary jobs and the AI engine/model.":L"\n\nLe nettoyage conserve les stems actifs/en préparation, les travaux temporaires, les fichiers inconnus et le moteur/modèle IA. Les morceaux source sont préservés. Pour supprimer aussi les stems actifs, choisir Son original ou Arrêter/libérer sur les deux platines, puis rouvrir ce gestionnaire.\nLa taille exclut les travaux temporaires et le moteur/modèle IA.";
            SendMessageW(window,TDM_SET_ELEMENT_TEXT,TDE_CONTENT,(LPARAM)text.c_str());SendMessageW(window,TDM_ENABLE_BUTTON,100,TRUE);SendMessageW(window,TDM_ENABLE_BUTTON,101,TRUE);
        }return S_OK;
    }
};
inline void manageCache(HWND owner,bool en,const std::set<std::wstring>& protectedKeys){
    CacheDialog state{en,protectedKeys};TASKDIALOG_BUTTON buttons[]={{100,en?L"Clear unused cache":L"Vider le cache inutilisé"},{101,en?L"Refresh size":L"Actualiser la taille"}};
    TASKDIALOGCONFIG config={sizeof(config)};config.hwndParent=owner;config.dwFlags=TDF_CALLBACK_TIMER|TDF_ALLOW_DIALOG_CANCELLATION|TDF_SIZE_TO_CONTENT;config.pszWindowTitle=L"DJ Decks";config.pszMainInstruction=en?L"Stem cache":L"Cache des stems";config.pszContent=en?L"Reading cache size…":L"Calcul de la taille du cache…";config.cButtons=2;config.pButtons=buttons;config.dwCommonButtons=TDCBF_CLOSE_BUTTON;config.pfCallback=CacheDialog::callback;config.lpCallbackData=(LONG_PTR)&state;
    if(FAILED(TaskDialogIndirect(&config,nullptr,nullptr,nullptr)))MessageBoxW(owner,en?L"Unable to open the stem cache manager.":L"Impossible d'ouvrir le gestionnaire de cache des stems.",L"DJ Decks",MB_OK|MB_ICONERROR);
}
}
