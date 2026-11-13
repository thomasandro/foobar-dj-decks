#pragma once
#include <cwchar>
#include <string>
namespace dj_language {
struct Entry { const wchar_t * fr; const wchar_t * en; };
inline constexpr Entry entries[] = {
    {L"Temps ici",L"Beat here"},
    {L"Tonalit\u00e9 fixe : OUI",L"Key lock: ON"},
    {L"Mode vinyle",L"Vinyl mode"},
    {L"Vitesse  %+.1f %%",L"Speed  %+.1f %%"},
    {L"LECTURE",L"PLAYING"}, {L"PR\u00caT",L"READY"}, {L"VIDE",L"EMPTY"},
    {L"Glisse un morceau ici",L"Drop a track here"},
    {L"\u25b6   LECTURE",L"\u25b6   PLAY"},
    {L"D\u00e9but",L"Start"},
    {L"\u25b6  Lancer A + B",L"\u25b6  Start A + B"},
    {L"\u21c4  Inverser A / B",L"\u21c4  Swap A / B"},
    {L"Grille manuelle | 1 trait / %u temps",L"Manual grid | 1 line / %u beats"},
    {L"Battements estim\u00e9s | 1 trait / %u temps",L"Estimated beats | 1 line / %u beats"},
    {L"Grille BPM indicative | 1 trait / %u temps",L"Indicative BPM grid | 1 line / %u beats"},
    {L"Grille manuelle",L"Manual grid"},
    {L"Battements estim\u00e9s",L"Estimated beats"},
    {L"Grille BPM indicative (phase incertaine)",L"Indicative BPM grid (uncertain phase)"},
    {L"Rep\u00e8res : tag BPM valide requis",L"Markers: valid BPM tag required"},
    {L"Sortie audio bloqu\u00e9e depuis %d s",L"Audio output stalled for %d s"},
    {L"Grille : tag BPM valide requis",L"Grid: valid BPM tag required"},
    {L"Charge les deux platines",L"Load both decks"},
    {L"Deux tags BPM valides requis",L"Two valid BPM tags required"},
    {L"BPM hors plage (-16 / +16 %)",L"BPM out of range (-16 / +16 %)"},
    {L"SYNC : analyse en cours ou rythme incertain",L"SYNC: analysis in progress or uncertain beat"},
    {L"SYNC : position incompatible",L"SYNC: incompatible position"},
    {L"SYNC : positionnement impossible",L"SYNC: seeking unavailable"},
    {L"Relecture impossible",L"Replay unavailable"},
    {L"Erreur au retour au d\u00e9but",L"Could not return to start"},
    {L"Positionnement impossible",L"Seeking unavailable"},
    {L"Sortie audio : erreur inconnue",L"Audio output: unknown error"},
    {L"Erreur : ",L"Error: "},
    {L"Positionnement : ",L"Seeking: "},
    {L"Sortie audio : ",L"Audio output: "}
};
inline const wchar_t * lookup(const wchar_t * text, bool english) {
    if (english) for (const auto & entry : entries) if (std::wcscmp(text,entry.fr)==0) return entry.en;
    return text;
}
inline std::wstring message(const std::wstring & text, bool english) {
    if (!english) return text;
    for (const auto & entry : entries) if (text == entry.fr) return entry.en;
    for (const auto & entry : entries) {
        const size_t length = std::wcslen(entry.fr);
        if (length && entry.fr[length-1] == L' ' && text.compare(0,length,entry.fr)==0)
            return std::wstring(entry.en) + text.substr(length);
    }
    return text;
}
}
