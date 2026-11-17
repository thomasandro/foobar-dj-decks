
int main(){
 _Module.Init(nullptr,GetModuleHandle(nullptr));
 CDjPanel panel;
 panel.SettingsChanged(1240,1);assert(cfg_outputRate.get()==44100&&panel.m_outputRatePreference==44100);panel.SettingsChanged(1240,0);assert(panel.m_outputRatePreference==0);
 panel.SettingsChanged(1230,1);assert(cfg_bpmProvider.get()==1);panel.SettingsChanged(1230,2);assert(cfg_bpmProvider.get()==2);panel.SettingsChanged(1230,0);assert(cfg_bpmProvider.get()==0);
 panel.SettingsChanged(1220,0);assert(!panel.m_limitEnabled&&cfg_masterLimiter.get()==0);panel.SettingsChanged(1220,1);assert(panel.m_limitEnabled);panel.SettingsChanged(1221,2);assert(panel.m_limitCeiling==-3&&cfg_masterCeiling.get()==-30);panel.SettingsChanged(1221,1);assert(panel.m_limitCeiling==-1);
 for(bool english:{false,true}){g_english=english;assert(dj_settings::tabs(english).size()==6);for(int page=0;page<6;++page){auto rows=panel.SettingsValues(page);assert(!rows.empty());std::vector<int> ids;for(auto& item:rows)if(item.id){assert(std::find(ids.begin(),ids.end(),item.id)==ids.end());ids.push_back(item.id);if(item.kind==dj_settings::Kind::combo)assert(item.value>=0&&item.value<(int)item.options.size());}}}
 panel.SettingsChanged(1000,0);assert(!g_english);panel.SettingsChanged(1000,1);assert(g_english);
 panel.SettingsChanged(1010,1);assert(dj_skin::flat);panel.SettingsChanged(1100,3);assert(dj_skin::colorIndex[0]==3);panel.SettingsChanged(1120,4);assert(dj_skin::colorIndex[1]==4);
 panel.SettingsChanged(1210,2);assert(g_replaygain==2);panel.SettingsChanged(1300,0);assert(cfg_queueFormat.get().value==dj_queue_format::presets[0]);panel.SettingsChanged(1300,4);assert(panel.commands.back()==304);
 panel.SettingsChanged(220,1);assert(g_scrobble);auto count=panel.commands.size();panel.SettingsChanged(220,1);assert(panel.commands.size()==count);
 panel.SettingsChanged(305,1);assert(cfg_queueNumbers.get()==1);panel.SettingsChanged(306,0);assert(panel.commands.back()==306);panel.SettingsChanged(200,0);assert(panel.commands.back()==200);
 panel.SettingsChanged(1402,73);assert(cfg_cueVolume.get()==73&&panel.m_cueVolume.load()==.73);panel.SettingsChanged(1403,4);assert(cfg_previewMode.get()==4);
 GUID backend={1},device={2};panel.m_settingsDevices={{L"Headphones",backend,device}};panel.SettingsChanged(1400,1);assert(cfg_cueBackend.get()==backend&&cfg_cueDevice.get()==device&&panel.configured>0);
 panel.SettingsChanged(1404,1);assert(panel.m_cue[0]);panel.SettingsChanged(1405,1);assert(panel.m_cue[1]);panel.SettingsChanged(1407,0);assert(!panel.m_cue[0]&&!panel.m_cue[1]);
 output_manager::get()->core={backend,device};count=panel.configured;panel.SettingsChanged(1400,1);assert(panel.configured==count&&panel.refreshed>0);output_manager::get()->core={};
 panel.SettingsChanged(1400,0);assert(cfg_cueBackend.get()==GUID{}&&!panel.m_cue[0]);
 panel.SettingsChanged(1500,6);assert(panel.m_fadeSeconds==10);panel.SettingsChanged(1501,1);assert(panel.m_linearFade);panel.SettingsChanged(1502,1);assert(panel.m_loopSnap[0]&&cfg_loopSnapA.get()==1);panel.SettingsChanged(1503,1);assert(panel.m_loopSnap[1]&&cfg_loopSnapB.get()==1);
 panel.SettingsChanged(1510,1);panel.SettingsChanged(1511,1);panel.SettingsChanged(1512,3);assert(cfg_trimHead.get()==1&&cfg_trimTail.get()==1&&cfg_trimDb.get()==-70&&panel.trimmed==3);
 panel.SettingsChanged(1604,0);assert(panel.distributed==1);panel.SettingsChanged(1600,0);assert(panel.exported==0);panel.SettingsChanged(1601,0);assert(panel.exported==1);panel.SettingsChanged(1602,0);assert(panel.imported==0);panel.SettingsChanged(1603,0);assert(panel.imported==1);
 // Merely rendering/opening the settings must not reset custom stored values.
 panel.m_fadeSeconds=7.5;cfg_trimDb=-77;cfg_queueFormat.value.value="custom";auto rows=panel.SettingsValues(4);assert(panel.m_fadeSeconds==7.5&&cfg_trimDb.get()==-77);rows=panel.SettingsValues(3);assert(rows[0].value==4&&cfg_queueFormat.get().value=="custom");
 CWindow owner;CRect rect(0,0,900,700);owner.Create(L"STATIC",nullptr,rect,L"Hidden owner",WS_OVERLAPPEDWINDOW);
 panel.SettingsChanged(1900,0);assert(!dj_skin::flat&&dj_skin::colorIndex[0]==0&&dj_skin::colorIndex[1]==1);
 panel.SettingsChanged(1240,1);panel.SettingsChanged(1901,0);assert(panel.m_outputRatePreference==0&&panel.m_limitEnabled&&panel.m_limitCeiling==-1.);
 panel.SettingsChanged(1903,0);assert(cfg_queueNumbers.get()==0&&std::string(cfg_queueFormat.get().get_ptr())==dj_queue_format::presets[0]);
 panel.SettingsChanged(1904,0);assert(panel.m_fadeSeconds==5&&!panel.m_linearFade&&cfg_trimDb.get()==-60);
 auto& window=panel.m_settings;window.values=[&](int page){return panel.SettingsValues(page);};window.changed=[&](int id,int value){panel.SettingsChanged(id,value);};
 window.open(owner,false,RGB(0,194,211),0,false);assert(window.m_hWnd&&!window.IsWindowVisible());auto original=window.m_hWnd;
 for(int page=0;page<6;++page){window.select(page);assert(window.selected==page);SendMessage(window.m_hWnd,WM_PAINT,0,0);}
 window.select(4);bool previousQ=panel.m_loopSnap[0];auto q=window.control(1502);assert((GetWindowLongPtr(q,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW);SendMessage(q,BM_CLICK,0,0);assert(panel.m_loopSnap[0]!=previousQ);
 HDC render=CreateCompatibleDC(nullptr);BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=600;info.bmiHeader.biHeight=-40;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;void* pixels=nullptr;auto image=CreateDIBSection(render,&info,DIB_RGB_COLORS,&pixels,nullptr,0);auto oldImage=SelectObject(render,image);
 DRAWITEMSTRUCT draw={};draw.CtlType=ODT_BUTTON;draw.CtlID=1502;draw.hwndItem=window.control(1502);draw.hDC=render;draw.rcItem={0,0,600,40};SendMessage(GetParent(draw.hwndItem),WM_DRAWITEM,1502,(LPARAM)&draw);GdiFlush();int bright=0;auto colors=(DWORD*)pixels;for(int y=0;y<40;++y)for(int x=40;x<600;++x){auto value=colors[y*600+x];if((value&255)>180&&((value>>8)&255)>180&&((value>>16)&255)>180)++bright;}assert(bright>20);SelectObject(render,oldImage);DeleteObject(image);DeleteDC(render);
 SendMessage(window.m_hWnd,WM_COMMAND,MAKEWPARAM(23,BN_CLICKED),0);assert(window.selected==3);
 window.select(0);auto language=window.control(1000);assert(language);SendMessage(language,CB_SETCURSEL,0,0);SendMessage(GetParent(language),WM_COMMAND,MAKEWPARAM(1000,CBN_SELCHANGE),(LPARAM)language);assert(!g_english&&window.control(1000));
 // Option changes with the same row schema retain HWNDs and keyboard focus.
 auto theme=window.control(1010);auto color=window.control(1100);SetFocus(theme);SendMessage(theme,CB_SETCURSEL,1,0);SendMessage(GetParent(theme),WM_COMMAND,MAKEWPARAM(1010,CBN_SELCHANGE),(LPARAM)theme);assert(window.control(1010)==theme&&window.control(1100)==color&&SendMessage(theme,CB_GETCURSEL,0,0)==1);
 window.select(0);assert(window.control(1010)==theme);assert(!window.IsWindowVisible());
 auto cue=panel.SettingsValues(2);assert(!cue.empty());window.select(2);assert(!IsWindowEnabled(window.control(1404)));
 window.select(4);window.SetWindowPos(nullptr,0,0,640,400,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);SendMessage(window.control(1512),WM_SETFOCUS,0,0);RECT controlRect,host;GetWindowRect(window.control(1512),&controlRect);GetWindowRect(GetParent(window.control(1512)),&host);assert(controlRect.top>=host.top&&controlRect.bottom<=host.bottom+window.px(230));
 // Actual scrollbar/wheel paths: no control recreation or accidental combo changes.
 window.select(1);window.SetWindowPos(nullptr,0,0,640,400,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
 auto viewport=window.viewport();auto rateControl=window.control(1240);auto selectedRate=SendMessage(rateControl,CB_GETCURSEL,0,0);
 SendMessage(viewport,WM_VSCROLL,SB_BOTTOM,0);assert(window.scrollOffset()>0);SendMessage(viewport,WM_VSCROLL,SB_TOP,0);assert(window.scrollOffset()==0);
 SendMessage(rateControl,WM_MOUSEWHEEL,MAKEWPARAM(0,(short)-60),0);assert(window.scrollOffset()==0);SendMessage(rateControl,WM_MOUSEWHEEL,MAKEWPARAM(0,(short)-60),0);assert(window.scrollOffset()>0&&window.control(1240)==rateControl&&SendMessage(rateControl,CB_GETCURSEL,0,0)==selectedRate);
 SendMessage(viewport,WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,120),0);assert(window.scrollOffset()==120);SendMessage(viewport,WM_VSCROLL,SB_LINEUP,0);assert(window.scrollOffset()==120-window.px(30));
 for(int n=0;n<80;++n){SendMessage(viewport,WM_VSCROLL,n%2?SB_PAGEUP:SB_PAGEDOWN,0);assert(window.control(1240)==rateControl);}SendMessage(window.m_hWnd,WM_MOUSEWHEEL,MAKEWPARAM(0,(short)120),0);
 window.open(owner,false,RGB(0,194,211),3,false);assert(window.m_hWnd==original&&window.selected==3&&!window.IsWindowVisible());
 window.close();assert(!window.m_hWnd&&IsWindow(owner));window.open(owner,false,RGB(0,194,211),0,false);assert(window.m_hWnd);owner.DestroyWindow();assert(!window.m_hWnd||!IsWindow(window.m_hWnd));if(window.m_hWnd)window.close();
 _Module.Term();std::cout<<"Hidden tabbed settings + actual bindings: 6 bilingual categories, current-value preservation, commands, cue master exclusion, trim/fade/Q, callbacks, reuse, resize, close/reopen and owned teardown passed; no host, device or screenshot used\n";
}

