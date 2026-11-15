param([ValidateSet('x64','x86')][string]$Architecture='x64')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$source=Get-Content -Raw (Join-Path $repo 'sdk/foobar2000/foo_dj/ui_element.cpp')
$output=Join-Path $repo 'build/tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$a=$source.IndexOf('void CDjPanel::SaveLoopPoints(');$b=$source.IndexOf('void CDjPanel::SeekDeck(', $a)
[IO.File]::WriteAllText((Join-Path $output 'loop-generated.cpp'),'#include "../../tests/seek_fixture.h"'+"`n"+$source.Substring($a,$b-$a)+"`n"+'#include "../../tests/loop_controls_cases.inc"',[Text.UTF8Encoding]::new($false))
$a=$source.IndexOf('static std::string ReadSessionFile(');$b=$source.IndexOf('void CDjPanel::SaveSession()', $a)
[IO.File]::WriteAllText((Join-Path $output 'loop-storage-io.inc'),$source.Substring($a,$b-$a),[Text.UTF8Encoding]::new($false))
$layoutFirst=$source.IndexOf('struct DeckLayout {');$layoutLast=$source.IndexOf('// Colonne du curseur', $layoutFirst)
$layout='static constexpr int kFaderHeight=52, kFooterHeight=0;'+"`n"+$source.Substring($layoutFirst,$layoutLast-$layoutFirst)
[IO.File]::WriteAllText((Join-Path $output 'skin-layout.inc'),$layout,[Text.UTF8Encoding]::new($false))
$vs=& (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe') -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$env:DJ_DECKS_VCVARS=Join-Path $vs ('VC/Auxiliary/Build/vcvars'+$(if($Architecture -eq 'x86'){'32'}else{'64'})+'.bat')
& cmd.exe /d /c (Join-Path $PSScriptRoot 'test-loop-storage.cmd')
if($LASTEXITCODE -ne 0){throw 'Loop persistence regression failed'}
