param([ValidateSet('x64','x86')][string]$Architecture='x64')
$ErrorActionPreference='Stop';$repo=Split-Path -Parent $PSScriptRoot;$output=Join-Path $repo 'build/tests';New-Item -ItemType Directory -Force -Path $output|Out-Null
$source=Get-Content -Raw (Join-Path $repo 'sdk/foobar2000/foo_dj/ui_element.cpp')
$a=$source.IndexOf('struct DeckLayout {');$b=$source.IndexOf('// Colonne du curseur', $a)
[IO.File]::WriteAllText((Join-Path $output 'skin-layout.inc'),'static constexpr int kFaderHeight=52,kFooterHeight=0;'+"`n"+$source.Substring($a,$b-$a),[Text.UTF8Encoding]::new($false))
$a=$source.IndexOf('void CDjPanel::UpdateTooltips(');$b=$source.IndexOf('void CDjPanel::PaintAll(', $a)
[IO.File]::WriteAllText((Join-Path $output 'tooltip-generated.cpp'),'#include "../../tests/tooltip_fixture.h"'+"`n"+$source.Substring($a,$b-$a)+"`n"+'#include "../../tests/tooltip_cases.inc"',[Text.UTF8Encoding]::new($false))
$a=$source.IndexOf('void CDjPanel::OnPaint(');$b=$source.IndexOf('void CDjPanel::UpdateTooltips(', $a)
$paint=$source.Substring($a,$b-$a).Replace('CDjPanel::OnPaint','PaintPanel::OnPaint')
$generated=Get-Content -Raw (Join-Path $output 'tooltip-generated.cpp')
$generated=$generated.Replace('#include "../../tests/tooltip_cases.inc"',$paint+"`n"+'#include "../../tests/tooltip_cases.inc"')
[IO.File]::WriteAllText((Join-Path $output 'tooltip-generated.cpp'),$generated,[Text.UTF8Encoding]::new($false))
$vs=& (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe') -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$env:DJ_DECKS_VCVARS=Join-Path $vs ('VC/Auxiliary/Build/vcvars'+$(if($Architecture -eq 'x86'){'32'}else{'64'})+'.bat')
& cmd /d /c (Join-Path $PSScriptRoot 'test-tooltips.cmd');if($LASTEXITCODE -ne 0){throw 'Tooltip regression failed'}
