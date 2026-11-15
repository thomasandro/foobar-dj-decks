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
$autoFirst = $source.IndexOf('void CDjPanel::CancelAutomation()')
$autoMenu = $source.IndexOf('void CDjPanel::AutomationMenu(', $autoFirst)
$autoTick = $source.IndexOf('void CDjPanel::AutomationTick()', $autoMenu)
$autoEnd = $source.IndexOf('void CDjPanel::TogglePlay(', $autoTick)
if ($autoFirst -lt 0 -or $autoMenu -lt $autoFirst -or $autoEnd -lt $autoTick) { throw 'Automatic playback test extraction anchors changed.' }
$auto = '#include "../../tests/auto_fixture.h"' + "`n" + $source.Substring($autoFirst,$autoMenu-$autoFirst) + $source.Substring($autoTick,$autoEnd-$autoTick) + "`n" + '#include "../../tests/auto_cases.inc"'
[IO.File]::WriteAllText((Join-Path $output 'auto-generated.cpp'),$auto,[Text.UTF8Encoding]::new($false))
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
$rgSource = [IO.File]::ReadAllText((Join-Path $repo 'sdk/foobar2000/SDK/replaygain.cpp'))
$rgFirst = $rgSource.IndexOf('audio_sample t_replaygain_config::query_scale(const replaygain_info & info) const {')
$rgLast = $rgSource.IndexOf('audio_sample t_replaygain_config::query_scale(const metadb_handle_ptr', $rgFirst)
if ($rgFirst -lt 0 -or $rgLast -lt $rgFirst) { throw 'ReplayGain extraction anchors changed.' }
$rg = '#include "../../tests/replaygain_fixture.h"' + "`n" + $rgSource.Substring($rgFirst,$rgLast-$rgFirst) + "`n" + '#include "../../tests/replaygain_cases.inc"'
[IO.File]::WriteAllText((Join-Path $output 'replaygain-generated.cpp'),$rg,[Text.UTF8Encoding]::new($false))
$scFirst=$source.IndexOf('void ScrobbleTick(){')
$scLast=$source.IndexOf('        void StemsTick();',$scFirst)
if($scFirst -lt 0 -or $scLast -lt $scFirst){throw 'Scrobble extraction anchors changed'}
$sc=$source.Substring($scFirst,$scLast-$scFirst).Replace('void ScrobbleTick(){','void CDjPanel::ScrobbleTick(){')
[IO.File]::WriteAllText((Join-Path $output 'scrobble-generated.cpp'),'#include "../../tests/scrobble_fixture.h"'+"`n"+$sc+"`n"+'#include "../../tests/scrobble_cases.inc"',[Text.UTF8Encoding]::new($false))
$metadataFirst=$source.IndexOf('void EnsureQueueInfo(){')
$metadataLast=$source.IndexOf('        int m_queueCurrent[2]',$metadataFirst)
if($metadataFirst -lt 0 -or $metadataLast -lt $metadataFirst){throw 'Metadata extraction anchors changed'}
$metadata=$source.Substring($metadataFirst,$metadataLast-$metadataFirst).Replace('void EnsureQueueInfo(){','void CDjPanel::EnsureQueueInfo(){')
[IO.File]::WriteAllText((Join-Path $output 'queue-metadata-generated.cpp'),'#include "../../tests/queue_metadata_fixture.h"'+"`n"+$metadata+"`n"+'#include "../../tests/queue_metadata_cases.inc"',[Text.UTF8Encoding]::new($false))
$labelFirst=$source.IndexOf('std::wstring CDjPanel::QueueLabel(')
$labelLast=$source.IndexOf('    void CDjPanel::UpdateQueue(', $labelFirst)
if($labelFirst -lt 0 -or $labelLast -lt $labelFirst){throw 'Queue label extraction anchors changed'}
[IO.File]::WriteAllText((Join-Path $output 'queue-label-generated.cpp'),'#include "../../tests/queue_label_fixture.h"'+"`n"+$source.Substring($labelFirst,$labelLast-$labelFirst)+"`n"+'#include "../../tests/queue_label_cases.inc"',[Text.UTF8Encoding]::new($false))
$prior = $env:DJ_DECKS_VCVARS
try {
    $env:DJ_DECKS_VCVARS = $vcvars
    & cmd.exe /d /c (Join-Path $PSScriptRoot 'test-native.cmd')
    if ($LASTEXITCODE -ne 0) { throw 'Native tests failed.' }
} finally { $env:DJ_DECKS_VCVARS = $prior }
Write-Output 'All native tests passed. No foobar host was launched.'
if ($env:DJ_STEM_ENGINE) {
    & (Join-Path $env:DJ_STEM_ENGINE 'venv/Scripts/python.exe') (Join-Path $repo 'tests/test-stem-backend.py')
    if ($LASTEXITCODE -ne 0) { throw 'Stem backend tests failed.' }
}
