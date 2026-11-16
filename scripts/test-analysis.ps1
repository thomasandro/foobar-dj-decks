param([ValidateSet('x64','x86')][string]$Architecture='x64')
$ErrorActionPreference='Stop';$repo=Split-Path -Parent $PSScriptRoot;$s=[IO.File]::ReadAllText((Join-Path $repo 'sdk/foobar2000/foo_dj/ui_element.cpp'))
$a=$s.IndexOf('void CDjPanel::AnalyseDeck(');$b=$s.IndexOf('void CDjPanel::LoadDeck(', $a)
$c=$s.IndexOf('void CDjPanel::QueueAction(');$d=$s.IndexOf('void CDjPanel::AddQueue(', $c)
if($a -lt 0 -or $b -lt $a -or $c -lt 0 -or $d -lt $c){throw 'Analysis/queue extraction anchors changed'}
[IO.File]::WriteAllText((Join-Path $repo 'build/tests/analysis-queue-generated.cpp'),'#include "../../tests/analysis_queue_fixture.h"'+"`n"+$s.Substring($a,$b-$a)+$s.Substring($c,$d-$c)+"`n"+'#include "../../tests/analysis_queue_cases.inc"',[Text.UTF8Encoding]::new($false))
$vs=& (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe') -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$env:DJ_DECKS_VCVARS=Join-Path $vs ('VC/Auxiliary/Build/vcvars'+$(if($Architecture -eq 'x86'){'32'}else{'64'})+'.bat')
& cmd.exe /d /c (Join-Path $PSScriptRoot 'test-analysis.cmd')
if($LASTEXITCODE -ne 0){throw 'Analysis/queue regression failed'}
