#pragma once
#include <string>
#include <cwctype>

namespace dj_key {
// Convert explicit major/minor keys; never guess an unknown or modal tag.
inline std::wstring camelot(std::wstring s) {
    std::wstring t;
    for (wchar_t c : s) {
        if (iswspace(c)) continue;
        if (c == L'\u266f') c = L'#';
        if (c == L'\u266d') c = L'b';
        t += c;
    }
    if (t.empty()) return {};
    size_t p = 0;
    while (p < t.size() && t[p] >= L'0' && t[p] <= L'9') ++p;
    if (p && p <= 2 && p + 1 == t.size()) {
        int n = std::stoi(t.substr(0,p));
        wchar_t mode = towlower(t[p]);
        if (n >= 1 && n <= 12) {
            if (mode == L'a' || mode == L'b') return std::to_wstring(n) + wchar_t(towupper(mode));
            if (mode == L'd' || mode == L'm') return std::to_wstring((n + 6) % 12 + 1) + (mode == L'm' ? L"A" : L"B");
        }
    }
    int pitch;
    switch (towupper(t[0])) {
    case L'C': pitch=0; break; case L'D': pitch=2; break;
    case L'E': pitch=4; break; case L'F': pitch=5; break;
    case L'G': pitch=7; break; case L'A': pitch=9; break;
    case L'B': pitch=11; break; default: return {};
    }
    p=1;
    if (p<t.size() && (t[p]==L'#' || t[p]==L'b')) { pitch += t[p]==L'#' ? 1 : -1; ++p; }
    std::wstring mode=t.substr(p);
    bool minor=mode==L"m"; // Uppercase M denotes major.
    for (auto &c:mode) c=towlower(c);
    if (mode==L"min" || mode==L"minor" || mode==L"mineur") minor=true;
    else if (!(mode.empty() || mode==L"maj" || mode==L"major" || mode==L"majeur" || (mode==L"m" && !minor))) {
        if (!minor) return {};
    }
    pitch=(pitch+12)%12;
    const int major[]={8,3,10,5,12,7,2,9,4,11,6,1};
    const int min[]={5,12,7,2,9,4,11,6,1,8,3,10};
    return std::to_wstring(minor?min[pitch]:major[pitch])+(minor?L"A":L"B");
}
inline std::wstring display(const std::wstring &tag) {
    auto value=camelot(tag);
    if (!value.empty()) return value;
    auto first=tag.find_first_not_of(L" \t\r\n");
    if (first==std::wstring::npos) return {};
    return tag.substr(first,tag.find_last_not_of(L" \t\r\n")-first+1);
}
inline std::wstring standard(const std::wstring& tag) {
    const auto c=camelot(tag);
    if(c.empty()) return {};
    const int n=std::stoi(c)-1;
    const wchar_t* minor[]={L"Abm",L"Ebm",L"Bbm",L"Fm",L"Cm",L"Gm",L"Dm",L"Am",L"Em",L"Bm",L"F#m",L"C#m"};
    const wchar_t* major[]={L"B",L"F#",L"Db",L"Ab",L"Eb",L"Bb",L"F",L"C",L"G",L"D",L"A",L"E"};
    return c.back()==L'A'?minor[n]:major[n];
}
inline std::wstring label(const std::wstring& tag) {
    const auto c=camelot(tag);
    return c.empty()?display(tag):c+L" ("+standard(c)+L")";
}
enum class Compatibility { unknown, usual, other };
inline std::wstring transpose(const std::wstring& tag,int semitones) {
    auto c=camelot(tag);
    if(c.empty()) return tag;
    int n=(std::stoi(c)-1+7*(semitones%12)+120)%12+1;
    return std::to_wstring(n)+c.back();
}
// Match the reference's effective key, retaining the destination's major/minor mode.
// Search absolute settings so existing transposition and the +/-12 range are respected.
inline bool match(const std::wstring& own,const std::wstring& reference,int current,int referenceShift,int& result) {
    const auto a=camelot(own), b=camelot(transpose(reference,referenceShift));
    if(a.empty() || b.empty() || current < -12 || current > 12) return false;
    const auto target=b.substr(0,b.size()-1)+a.back();
    bool found=false; int best=0;
    for(int s=-12;s<=12;++s) if(transpose(a,s)==target) {
        const int distance=std::abs(s-current), previous=std::abs(best-current);
        if(!found || distance<previous || (distance==previous && std::abs(s)<std::abs(best))) {best=s;found=true;}
    }
    if(found) result=best;
    return found;
}
inline Compatibility compatibility(const std::wstring& a,const std::wstring& b,bool pitchStable=true) {
    const auto x=camelot(a), y=camelot(b);
    if(!pitchStable || x.empty() || y.empty()) return Compatibility::unknown;
    const int nx=std::stoi(x), ny=std::stoi(y);
    const int delta=nx>ny?nx-ny:ny-nx;
    return nx==ny || (x.back()==y.back() && (delta==1 || delta==11))
        ? Compatibility::usual : Compatibility::other;
}
}
