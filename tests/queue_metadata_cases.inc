int main(){
    auto api=metadb_io_v2::get();
    {CDjPanel p;auto native=metadb::get()->handle_create("file://native",2);native->known=true;native->seconds=238;p.m_dropHandles.add_item(native);std::weak_ptr<H> weak=native;native.reset();
     p.m_queue[0]={{"file://native",2}};p.EnsureQueueInfo();p.m_dropHandles.remove_all();assert(!weak.expired()&&api->calls==0&&p.m_queueHandles[0][0]->seconds==238);
     p.m_queue[0].clear();p.m_queue[1]={{"file://native",2}};p.EnsureQueueInfo();assert(!weak.expired()&&p.m_queueHandles[1][0]->known&&api->calls==0);}
    {CDjPanel p;p.m_queue[0]={{"file://unknown",0},{"file://unknown",0},{"http://stream",0}};p.EnsureQueueInfo();assert(api->calls==1&&p.pending&&api->requested.get_count()==1&&api->mode==metadb_io::load_info_force);
     p.EnsureQueueInfo();assert(api->calls==1);p.orphan_task(1);p.EnsureQueueInfo();assert(api->calls==1); // failed/offline must not spin
     p.m_queueInfoAttempted.clear();p.m_forceQueueInfo=true;p.EnsureQueueInfo();assert(api->calls==2&&api->mode==metadb_io::load_info_force);
     p.orphan_task(1);p.m_queue[0].clear();p.m_queue[1].clear();p.EnsureQueueInfo();assert(p.m_queueInfoAttempted.empty());}
    {CDjPanel p;auto h=metadb::get()->handle_create("file://duration",4);h->known=true;h->seconds=0;p.m_queue[1]={{"file://duration",4}};p.EnsureQueueInfo();assert(api->calls==3&&api->requested[0]->get_subsong_index()==4);
     p.orphan_task(1);h->seconds=1027;p.EnsureQueueInfo();assert(api->calls==3);p.m_queueInfoAttempted.clear();p.m_forceQueueInfo=true;p.EnsureQueueInfo();assert(api->calls==4);}
    {CDjPanel p;p.m_queue[0]={{"file://fail",0}};api->fail=true;p.EnsureQueueInfo();assert(!p.pending&&api->calls==5);p.EnsureQueueInfo();assert(api->calls==5);api->fail=false;}
    {CDjPanel p;auto h=metadb::get()->handle_create("file://partial-tags",0);h->known=true;h->seconds=238;h->complete=false;p.m_queue[0]={{"file://partial-tags",0}};p.EnsureQueueInfo();assert(api->calls==6&&api->mode==metadb_io::load_info_force);}
    std::cout<<"Actual EnsureQueueInfo: native handle retention, A/B transfer lifetime, missing metadata/duration, dedup/subsongs, async serialization, no retry loop, force reload, pruning and read-failure cleanup passed (mock IO, no user files read)\n";
}
