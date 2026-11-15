#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
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

#include <atomic>
#include "../sdk/foobar2000/foo_dj/foo_dj_version.h"
#include "../sdk/foobar2000/foo_dj/tooltip_policy.h"
#include "../build/tests/skin-layout.inc"
namespace core_api{inline HINSTANCE get_my_instance(){return GetModuleHandle(nullptr);}}
inline std::atomic<bool> g_english{false};
struct CDjPanel{
 struct Tip{CRect rect;std::wstring text;};HWND m_hWnd=nullptr,m_tooltip=nullptr;CRect m_tipClient={0,0,0,0};bool m_tipEnglish=false;std::vector<Tip>m_tips;
 void UpdateTooltips(const CRect&);
};
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"comctl32.lib")

inline constexpr COLORREF kCanvas=RGB(14,18,24);
class PaintPanel:public CWindowImpl<PaintPanel>{
public:
 DECLARE_WND_CLASS_EX(L"foo_dj_hidden_paint_regression_v1",0,(-1))
 CBitmap m_paintBuffer;int m_paintWidth=0,m_paintHeight=0,painted=0;
 void PaintAll(CDCHandle dc,const CRect& r){++painted;dc.FillSolidRect(r,kCanvas);}
 void OnPaint(CDCHandle);
 BEGIN_MSG_MAP_EX(PaintPanel)
 MSG_WM_PAINT(OnPaint)
 END_MSG_MAP()
};
