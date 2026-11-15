int main(){
    CDjPanel panel;dj_queue::Item item{"file://wrong-fallback-name.opus",0};
    assert(panel.QueueLabel(item)==L"Artiste é - Titre (17:07)");
    metadb::get()->handle->seconds=238;assert(panel.QueueLabel(item)==L"Artiste é - Titre (3:58)");
    metadb::get()->handle->seconds=3605;assert(panel.QueueLabel(item)==L"Artiste é - Titre (1:00:05)");
    metadb::get()->handle->seconds=0;assert(panel.QueueLabel(item)==L"Artiste é - Titre (?)");
    cfg_queueFormat.script="%artist% - %title%";assert(panel.QueueLabel(item)==L"Artiste é - Titre");
    std::cout<<"Actual QueueLabel -> formatter callback -> actual duration hook -> Unicode label passed with SDK null-terminated names (mock formatting engine, no host)\n";
}
