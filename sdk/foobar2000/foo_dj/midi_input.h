#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <array>
#include <vector>
#include <string>
#include <atomic>
#pragma comment(lib,"winmm.lib")
namespace dj_midi {
struct Device {UINT id;std::wstring name;std::string key;};
inline std::string utf8(const std::wstring& s){int n=WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0,nullptr,nullptr);std::string out(n,0);if(n)WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),out.data(),n,nullptr,nullptr);return out;}
inline std::vector<Device> devices(){std::vector<Device> out;for(UINT i=0;i<midiInGetNumDevs();++i){MIDIINCAPSW caps={};if(midiInGetDevCapsW(i,&caps,sizeof(caps))!=MMSYSERR_NOERROR)continue;auto key=std::to_string(caps.wMid)+":"+std::to_string(caps.wPid)+":"+utf8(caps.szPname);int ordinal=0;for(auto& d:out)if(d.key.rfind(key+"#",0)==0)++ordinal;out.push_back({i,caps.szPname,key+"#"+std::to_string(ordinal)});}return out;}
// The driver callback only enqueues short messages in bounded storage. No UI,
// audio, allocation, file IO or WinMM calls are made inside it.
class Input {
 inline static std::atomic<Input*> owner{nullptr};
 HMIDIIN handle=nullptr;CRITICAL_SECTION lock;std::array<unsigned,256> queue{};size_t head=0,size=0;bool accepting=false;std::atomic<bool> overflow{false};
 static void CALLBACK callback(HMIDIIN,UINT msg,DWORD_PTR instance,DWORD_PTR packed,DWORD_PTR){if(msg!=MIM_DATA&&msg!=MIM_MOREDATA)return;auto self=reinterpret_cast<Input*>(instance);EnterCriticalSection(&self->lock);if(self->accepting){if(self->size<self->queue.size()){self->queue[(self->head+self->size)%self->queue.size()]=(unsigned)packed;++self->size;}else self->overflow=true;}LeaveCriticalSection(&self->lock);}
public:
 Input(){InitializeCriticalSection(&lock);}~Input(){close();DeleteCriticalSection(&lock);}Input(const Input&)=delete;Input& operator=(const Input&)=delete;
 bool opened()const{return handle!=nullptr;}
 void clear(){EnterCriticalSection(&lock);head=size=0;overflow=false;LeaveCriticalSection(&lock);}
 void close(){EnterCriticalSection(&lock);accepting=false;LeaveCriticalSection(&lock);if(handle){midiInStop(handle);midiInReset(handle);midiInClose(handle);handle=nullptr;}clear();auto expected=this;owner.compare_exchange_strong(expected,nullptr);}
 MMRESULT open(UINT id){close();Input* expected=nullptr;if(!owner.compare_exchange_strong(expected,this))return MMSYSERR_ALLOCATED;auto result=midiInOpen(&handle,id,(DWORD_PTR)&callback,(DWORD_PTR)this,CALLBACK_FUNCTION|MIDI_IO_STATUS);if(result!=MMSYSERR_NOERROR){handle=nullptr;close();return result;}EnterCriticalSection(&lock);accepting=true;LeaveCriticalSection(&lock);result=midiInStart(handle);if(result!=MMSYSERR_NOERROR)close();return result;}
 std::vector<unsigned> drain(bool& lost){std::array<unsigned,256> copy{};size_t n;EnterCriticalSection(&lock);n=size;for(size_t i=0;i<n;++i)copy[i]=queue[(head+i)%queue.size()];head=size=0;lost=overflow.exchange(false);LeaveCriticalSection(&lock);return {copy.begin(),copy.begin()+n};}
};
}
