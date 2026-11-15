
int main(){
 _Module.Init(nullptr,GetModuleHandle(nullptr));INITCOMMONCONTROLSEX init={sizeof(init),ICC_WIN95_CLASSES};InitCommonControlsEx(&init);
 CDjPanel panel;panel.m_hWnd=CreateWindowExW(0,L"STATIC",L"Hidden tooltip owner",WS_OVERLAPPEDWINDOW,0,0,900,700,nullptr,nullptr,GetModuleHandle(nullptr),nullptr);assert(panel.m_hWnd&&!IsWindowVisible(panel.m_hWnd));
 CRect rect(0,0,900,700);panel.UpdateTooltips(rect);assert(panel.m_tooltip);auto tip=panel.m_tooltip;auto count=SendMessage(tip,TTM_GETTOOLCOUNT,0,0);std::cout<<"Registered tooltip tools: "<<count<<" / "<<panel.m_tips.size()<<"\n";assert(count==(LRESULT)panel.m_tips.size()&&count>60);
 auto style=GetWindowLongPtr(tip,GWL_STYLE);assert((style&TTS_NOANIMATE)&&(style&TTS_NOFADE));assert(SendMessage(tip,TTM_GETDELAYTIME,TTDT_INITIAL,0)==300);assert(SendMessage(tip,TTM_GETDELAYTIME,TTDT_RESHOW,0)==75);
 auto text=panel.m_tips[0].text;auto address=panel.m_tips.data();for(int n=0;n<100;++n)panel.UpdateTooltips(rect);assert(panel.m_tooltip==tip&&panel.m_tips.data()==address&&SendMessage(tip,TTM_GETTOOLCOUNT,0,0)==count);
 g_english=true;panel.UpdateTooltips(rect);assert(panel.m_tips[0].text!=text&&SendMessage(tip,TTM_GETTOOLCOUNT,0,0)==count);
 panel.UpdateTooltips(CRect(0,0,1280,800));assert(SendMessage(tip,TTM_GETTOOLCOUNT,0,0)==count);
 RECT button={10,20,80,60};auto dirty=dj_tooltip::feedback(button);assert(dirty.left==4&&dirty.top==14&&dirty.right==86&&dirty.bottom==66);
 PaintPanel paint;CRect paintRect(0,0,600,400);assert(paint.Create(nullptr,paintRect,L"Hidden paint",WS_OVERLAPPEDWINDOW));assert(!paint.IsWindowVisible());SendMessage(paint.m_hWnd,WM_PAINT,0,0);auto buffer=paint.m_paintBuffer.m_hBitmap;assert(buffer&&paint.painted>0);SendMessage(paint.m_hWnd,WM_PAINT,0,0);assert(paint.m_paintBuffer.m_hBitmap==buffer);paint.SetWindowPos(nullptr,0,0,800,600,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);SendMessage(paint.m_hWnd,WM_PAINT,0,0);CRect client;paint.GetClientRect(&client);assert(paint.m_paintWidth==client.Width()&&paint.m_paintHeight==client.Height());paint.DestroyWindow();
 DestroyWindow(tip);DestroyWindow(panel.m_hWnd);_Module.Term();std::cout<<"Actual tooltip setup: 300/75ms delays, animation/fade disabled, stable registration across 100 paints, language/resize updates and bounded feedback region passed (hidden test owner only)\n";
}
