param([string]$VSWHERE = '')
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $VSWHERE) { $VSWHERE = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe' }
$vs = & $VSWHERE -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'C++ Visual Studio installation not found.' }
$vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $vcvars)) { throw 'vcvars64.bat not found.' }
$output = Join-Path $repo 'build\tests'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$source = [IO.File]::ReadAllText((Join-Path $repo 'sdk\foobar2000\foo_dj\ui_element.cpp'))
$first = $source.IndexOf('static void ResampleStep(')
$last = $source.IndexOf('// Use foobar''s front-cover', $first)
if ($first -lt 0 -or $last -lt $first) { throw 'Audio fixture extraction anchors changed.' }
$generated = '#include "../../tests/audio_fixture.h"' + "`n" + $source.Substring($first,$last-$first) + "`n" + '#include "../../tests/audio_cases.inc"' + "`n"
[IO.File]::WriteAllText((Join-Path $output 'audio-generated.cpp'),$generated,[Text.UTF8Encoding]::new($false))
$layoutFirst = $source.IndexOf('struct DeckLayout {')
$layoutLast = $source.IndexOf('// Colonne du curseur', $layoutFirst)
if ($layoutFirst -lt 0 -or $layoutLast -lt $layoutFirst) { throw 'Skin layout extraction anchors changed.' }
$layout = 'static constexpr int kFaderHeight=52, kFooterHeight=0;' + "`n" + $source.Substring($layoutFirst,$layoutLast-$layoutFirst)
[IO.File]::WriteAllText((Join-Path $output 'skin-layout.inc'),$layout,[Text.UTF8Encoding]::new($false))
$prior = $env:DJ_DECKS_VCVARS
try {
    $env:DJ_DECKS_VCVARS = $vcvars
    & cmd.exe /d /c (Join-Path $PSScriptRoot 'test-native.cmd')
    if ($LASTEXITCODE -ne 0) { throw 'Native tests failed.' }
} finally { $env:DJ_DECKS_VCVARS = $prior }
Write-Output 'All native tests passed. No foobar host was launched.'
