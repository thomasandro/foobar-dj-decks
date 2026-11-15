#include "../sdk/foobar2000/foo_dj/scrobble_detection.h"
#include <cassert>
#include <iostream>
int main(){
    using namespace dj_scrobble;
    assert(componentMatches("foo_scrobble","Scrobble"));
    assert(componentMatches("FOO_SCROBBLE.DLL",""));
    assert(componentMatches("C:\\components\\foo_scrobble.dll",""));
    assert(componentMatches(" /components/foo_scrobble.dll \r\n",""));
    assert(componentMatches("","Scrobble"));
    assert(!componentMatches("foo_scrobbler.dll","Other scrobbler"));
    assert(supportedVersion("1.6.0.22456")&&supportedVersion("1.6.0+build")&&supportedVersion(" 1.6.0.22456\r\n"));
    assert(!supportedVersion("1.60.0")&&!supportedVersion("1.7.0")&&!supportedVersion("2.6.0")&&!supportedVersion(""));
    std::cout<<"Scrobble detection: extensionless names, DLL paths/case/whitespace, name fallback and strict 1.6.x version gate passed\n";
}
