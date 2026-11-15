#pragma once
#include <string>
#include <algorithm>
#include <cctype>
namespace dj_scrobble {
inline std::string trimmed(std::string s){auto first=s.find_first_not_of(" \t\r\n");if(first==std::string::npos)return {};auto last=s.find_last_not_of(" \t\r\n");return s.substr(first,last-first+1);}
inline std::string moduleName(std::string s){s=trimmed(s);auto n=s.find_last_of("/\\");if(n!=std::string::npos)s.erase(0,n+1);std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return (char)std::tolower(c);});if(s.size()>=4&&s.substr(s.size()-4)==".dll")s.resize(s.size()-4);return s;}
inline bool supportedVersion(std::string s){s=trimmed(s);return s.rfind("1.6.",0)==0;}
inline bool componentMatches(const std::string& file,const std::string& name){return moduleName(file)=="foo_scrobble"||(trimmed(name)=="Scrobble");}
}
