param([ValidateSet('x64','x86')][string]$Architecture='x64')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$output=Join-Path $repo 'build/tests';New-Item -ItemType Directory -Force -Path $output|Out-Null
$source=Get-Content -Raw (Join-Path $repo 'sdk/foobar2000/foo_dj/ui_element.cpp')
$a=$source.IndexOf('    double CDjPanel::MidiCurrent(');$b=$source.IndexOf('    void CDjPanel::MidiInitialize(', $a)
if($a -lt 0 -or $b -lt $a){throw 'MIDI test anchors changed'}
[IO.File]::WriteAllText((Join-Path $output 'midi-apply.inc'),$source.Substring($a,$b-$a),[Text.UTF8Encoding]::new($false))
$vs=& (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe') -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
$env:DJ_DECKS_VCVARS=Join-Path $vs ('VC/Auxiliary/Build/vcvars'+$(if($Architecture -eq 'x86'){'32'}else{'64'})+'.bat')
& cmd.exe /d /c (Join-Path $PSScriptRoot 'test-midi.cmd')
if($LASTEXITCODE -ne 0){throw 'MIDI regression failed'}
