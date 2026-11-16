int main(){
 const GUID smart={0x51fa6f82,0x7669,0x4090,{0x99,0x7c,0x85,0x07,0xc6,0xec,0x2e,0x30}},classic={0x518d0234,0x3134,0x4fea,{0xbd,0x43,0x8f,0x61,0xee,0x3a,0x35,0x3b}},key={0xefb0d751,0x1a4e,0x4ea3,{0x9d,0x22,0x68,0x25,0x64,0xe7,0xad,0xca}};
 contextmenu_item commands;services={&commands};
 for(bool hasSmart:{false,true})for(bool hasClassic:{false,true})for(int preference:{0,1,2}){
  commands.ids.clear();if(hasSmart)commands.ids.push_back(smart);if(hasClassic)commands.ids.push_back(classic);cfg_bpmProvider.v=preference;releases=0;menu_helpers::calls=0;
  CDjPanel p;p.AnalyseDeck(0,0);int expected=dj_bpm::select(preference,hasSmart,hasClassic);
  assert(releases==(expected<0?0:1)&&menu_helpers::calls==(expected<0?0:1));if(expected>0){assert(menu_helpers::last==(expected==1?smart:classic));assert(menu_helpers::track.path=="test://track"&&menu_helpers::track.subsong==7);}
 }
 commands.ids={key};CDjPanel p;releases=0;p.AnalyseDeck(0,1);assert(menu_helpers::last==key&&releases==1);
 p.m_pendingLoad[0]=true;releases=0;p.AnalyseDeck(0,0);assert(releases==0);p.m_pendingLoad[0]=false;p.AnalyseDeck(1,0);assert(releases==0);
 p.AnalyseDeck(0,2);assert(releases==0);commands.ids={smart};cfg_bpmProvider.v=0;menu_helpers::throws=true;int refresh=p.refreshed;p.AnalyseDeck(0,0);assert(p.refreshed==refresh+1);menu_helpers::throws=false;
 menu_helpers::succeeds=false;int notices=popup_message::count;p.AnalyseDeck(0,0);assert(popup_message::count==notices+1);menu_helpers::succeeds=true;
 // Actual queue action: cancel is a strict no-op; clear ignores selection and preserves other list/loaded transport.
 CDjPanel q;q.m_queue[0]={{"one",1},{"two",2}};q.m_queue[1]={{"other",3}};q.m_queueCurrent[0]=1;q.m_previewOwned=true;q.m_previewDeck=0;
 answer=IDCANCEL;q.QueueAction(0,5,{},0);assert(q.m_queue[0].size()==2&&q.cancelled==0&&q.saved==0);
 answer=IDOK;q.QueueAction(0,5,{},0);assert(q.m_queue[0].empty()&&q.m_queueCurrent[0]==-1&&q.m_queue[1].size()==1&&q.loaded==0&&q.cancelled==1&&q.dropped==1&&q.stopped==1&&q.saved==1&&q.updated==1&&!q.m_chain[0]&&q.m_chain[1]);
 int count=questions;q.QueueAction(0,5,{},0);assert(questions==count&&q.saved==1);
 std::cout<<"Actual analyser dispatch and queue clear: provider availability/preference, GUID/subsong, release-before-call, refresh/errors, cancel/no-op, clear without selection, no loaded-track mutation and other-list preservation passed (mock services, no host/files)\n";
}
