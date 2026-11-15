#include <memory>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <iostream>
#include <stdexcept>
using t_size=size_t;
struct file_info_impl{bool tags=true;const char* meta_get(const char*,unsigned)const{return tags?"known tag":nullptr;}};
struct Item{std::string path;unsigned subsong;};
struct H {std::string path;unsigned sub;bool known=false,complete=true;double seconds=0;const char* get_path(){return path.c_str();}unsigned get_subsong_index(){return sub;}bool get_info(file_info_impl& info){info.tags=complete;return known;}double get_length(){return seconds;}};
using metadb_handle_ptr=std::shared_ptr<H>;
struct metadb_handle_list{std::vector<metadb_handle_ptr> data;void add_item(metadb_handle_ptr h){data.push_back(h);}size_t get_count()const{return data.size();}metadb_handle_ptr operator[](size_t n)const{return data[n];}void remove_all(){data.clear();}};
struct metadb{std::map<std::string,std::weak_ptr<H>> data;static metadb* get(){static metadb m;return &m;}metadb_handle_ptr handle_create(const char* path,unsigned sub){auto key=std::string(path)+std::to_string(sub);auto h=data[key].lock();if(!h){h=std::make_shared<H>();h->path=path;h->sub=sub;data[key]=h;}return h;}};
inline std::string GridKey(const std::string& p,unsigned s){return p+"#"+std::to_string(s);}
namespace filesystem{struct Path{bool empty;bool is_empty()const{return empty;}};inline Path g_get_native_path(const char* p){return {std::string(p).rfind("file://",0)!=0};}}
struct metadb_io{enum{load_info_default=0,load_info_force=1};};
struct metadb_io_v2{enum{op_flag_silent=16,op_flag_no_errors=1};int calls=0,mode=-1;bool fail=false;metadb_handle_list requested;static metadb_io_v2* get(){static metadb_io_v2 m;return &m;}void load_info_async(const metadb_handle_list& list,int type,void*,int,int){++calls;requested=list;mode=type;if(fail)throw std::runtime_error("mock read failure");}};
namespace console{inline void print(const char*){}}
class CDjPanel {
public:
    std::vector<Item> m_queue[2];metadb_handle_list m_queueHandles[2],m_dropHandles;std::set<std::string> m_queueInfoAttempted;bool m_forceQueueInfo=false,pending=false;
    bool have_task(int){return pending;}int create_task(int){pending=true;return 1;}void orphan_task(int){pending=false;}
    void EnsureQueueInfo();
};
