param([ValidateSet('x64','x86')][string]$Architecture='x64')
$ErrorActionPreference='Stop';$repo=Split-Path -Parent $PSScriptRoot;$output=Join-Path $repo 'build/tests';New-Item -ItemType Directory -Force -Path $output|Out-Null
$source=Get-Content -Raw (Join-Path $repo 'sdk/foobar2000/foo_dj/ui_element.cpp');$a=$source.IndexOf('dj_session::Bundle CDjPanel::CaptureBundle(');$b=$source.IndexOf('static bool BundleFileDialog(', $a)
[IO.File]::WriteAllText((Join-Path $output 'session-generated.cpp'),'#include "../../tests/session_bundle_fixture.h"'+"`n"+$source.Substring($a,$b-$a)+"`n"+'#include "../../tests/session_bundle_cases.inc"',[Text.UTF8Encoding]::new($false))
$prepFirst=$source.IndexOf('    struct PreparedTrack{');$prepLast=$source.IndexOf('// Position de toutes les zones', $prepFirst)
$membersFirst=$source.IndexOf('        struct PrepareRequest{');$membersLast=$source.IndexOf('        void PreparationTick();', $membersFirst)
$loadFirst=$source.IndexOf('    void CDjPanel::LoadDeck(');$loadLast=$source.IndexOf('    void CDjPanel::CommitLoadDeck(', $loadFirst)
[IO.File]::WriteAllText((Join-Path $output 'preparation-generated.cpp'),'#include "../../tests/preparation_fixture.h"'+"`n"+$source.Substring($prepFirst,$prepLast-$prepFirst)+"`n"+'#include "../../tests/preparation_panel_fixture.h"'+"`n"+$source.Substring($membersFirst,$membersLast-$membersFirst)+"`n};`n"+$source.Substring($loadFirst,$loadLast-$loadFirst)+"`n"+'#include "../../tests/preparation_cases.inc"',[Text.UTF8Encoding]::new($false))
$vs=& (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe') -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$env:DJ_DECKS_VCVARS=Join-Path $vs ('VC/Auxiliary/Build/vcvars'+$(if($Architecture -eq 'x86'){'32'}else{'64'})+'.bat')
& cmd /d /c (Join-Path $PSScriptRoot 'test-session-bundle.cmd');if($LASTEXITCODE -ne 0){throw 'Session bundle tests failed'}
