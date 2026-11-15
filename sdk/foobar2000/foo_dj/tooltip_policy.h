#pragma once
#include <windows.h>
#include <commctrl.h>
namespace dj_tooltip {
inline constexpr DWORD style=WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX|TTS_NOANIMATE|TTS_NOFADE;
inline constexpr int initial=300,reshow=75,autopop=14000;
inline RECT feedback(RECT rect){if(rect.right>rect.left&&rect.bottom>rect.top)InflateRect(&rect,6,6);return rect;}
}
