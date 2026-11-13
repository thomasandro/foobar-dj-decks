#include "../sdk/foobar2000/foo_dj/dj_language.h"
#include <cassert>
#include <iostream>
#include <vector>
std::vector<std::wstring> placeholders(const wchar_t* text){std::vector<std::wstring> out;for(size_t i=0;text[i];++i)if(text[i]==L'%'){size_t start=i;while(text[i+1]&&text[i+1]!=L'%'&&text[i+1]!=L'u'&&text[i+1]!=L'd'&&text[i+1]!=L'f'&&text[i+1]!=L's'&&text[i+1]!=L')')++i;if(text[i+1])++i;out.emplace_back(text+start,i-start+1);}return out;}
int main(){using namespace dj_language;
 for(const auto &entry:entries){assert(std::wstring(lookup(entry.fr,false))==entry.fr);assert(std::wstring(lookup(entry.fr,true))==entry.en);assert(placeholders(entry.fr)==placeholders(entry.en));}
 assert(std::wstring(lookup(L"BPM = B",true))==L"BPM = B");
 assert(message(L"Erreur : fichier absent",true)==L"Error: fichier absent");
 assert(message(L"SYNC : analyse en cours ou rythme incertain",true)==L"SYNC: analysis in progress or uncertain beat");
 assert(message(L"Sortie audio : erreur inconnue",true)==L"Audio output: unknown error");
 assert(message(L"Titre original",true)==L"Titre original");
 std::wcout<<L"French/English dictionary passed: labels, format placeholders, active errors and original metadata\n";
}
