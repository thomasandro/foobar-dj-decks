#define NOMINMAX
#include <windows.h>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include "../sdk/foobar2000/foo_dj/picojson.h"
#include "../sdk/foobar2000/foo_dj/beat_sync.h"
static std::wstring SessionFile(const char*name){return std::wstring(name,name+strlen(name));}
static std::string ReadSessionFile(const char * name = "foo_dj-session.json") {
		const auto path = SessionFile(name);
		HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) return {};
		const DWORD size = GetFileSize(file, nullptr);
		std::string text;
		if (size != INVALID_FILE_SIZE && size <= 4 * 1024 * 1024) {
			text.resize(size); DWORD read = 0;
			if (!ReadFile(file, text.data(), size, &read, nullptr) || read != size) text.clear();
		}
		CloseHandle(file); return text;
	}
	static bool WriteSessionFile(const std::string & text, const char * name = "foo_dj-session.json") {
		const auto path = SessionFile(name), temp = path + L".tmp";
		HANDLE file = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) return false;
		DWORD written = 0;
		const bool ok = WriteFile(file, text.data(), (DWORD)text.size(), &written, nullptr) && written == text.size() && FlushFileBuffers(file);
		CloseHandle(file);
		return ok && MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

int main(){
 picojson::object grids; const std::string first="[\"file:///music/été.opus\",0]",second="[\"file:///music/été.opus\",1]";
 grids[first]=picojson::value(dj_beats::manual(120,17.237).offset);grids[second]=picojson::value(0.111);
 assert(WriteSessionFile(picojson::value(grids).serialize(),"test-grid-save.json"));
 picojson::value saved;assert(picojson::parse(saved,ReadSessionFile("test-grid-save.json")).empty());assert(saved.is<picojson::object>());
 const auto restored=saved.get<picojson::object>();assert(restored.size()==2);assert(std::abs(restored.at(first).get<double>()-0.237)<1e-9);assert(restored.at(second).get<double>()==0.111);
 grids.erase(first);assert(WriteSessionFile(picojson::value(grids).serialize(),"test-grid-save.json"));assert(picojson::parse(saved,ReadSessionFile("test-grid-save.json")).empty());assert(saved.get<picojson::object>().count(first)==0&&saved.get<picojson::object>().count(second)==1);
 DeleteFileW(L"test-grid-save.json");
 std::cout<<"Grid-file persistence passed: Unicode paths, separate subsongs, overwrite and Auto reset\n";
}
