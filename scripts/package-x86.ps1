param([switch]$SkipBuild, [switch]$SkipTests)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build-x86.ps1') }
if (-not $SkipTests) { & (Join-Path $PSScriptRoot 'test.ps1') -Architecture x86 }
$versionText = Get-Content -Raw -LiteralPath (Join-Path $repo 'sdk\foobar2000\foo_dj\foo_dj_version.h')
if ($versionText -notmatch '#define FOO_DJ_VERSION "([0-9A-Za-z.-]+)"') { throw 'Version not found.' }
$version = $Matches[1]
$dll = Join-Path $repo 'build\bin\foo_dj.dll'
if (-not (Test-Path -LiteralPath $dll)) { throw 'Build first.' }
$bytes = [IO.File]::ReadAllBytes($dll)
$pe = [BitConverter]::ToInt32($bytes,0x3c)
if ([BitConverter]::ToUInt16($bytes,$pe+4) -ne 0x14c) { throw 'Not an x86 PE binary.' }
if (-not [Text.Encoding]::ASCII.GetString($bytes).Contains($version)) { throw 'Built DLL version differs from source version.' }
$dist = Join-Path $repo 'dist'
New-Item -ItemType Directory -Path $dist -Force | Out-Null
Add-Type -AssemblyName System.IO.Compression.FileSystem
$component = Join-Path $dist "foo_dj-$version-win-x86.fb2k-component"
$sourceZip = Join-Path $dist "foo_dj-$version-source.zip"
function New-ZipFile([string]$path) {
    $stream = [IO.File]::Open($path,[IO.FileMode]::Create,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
    return [IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create,$false)
}
function Add-File($zip,[string]$path,[string]$entry) {
    [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,$path,$entry,[IO.Compression.CompressionLevel]::Optimal) | Out-Null
}
function Add-Text($zip,[string]$entry,[string]$text) {
    $item = $zip.CreateEntry($entry)
    $writer = [IO.StreamWriter]::new($item.Open(),[Text.UTF8Encoding]::new($false))
    try { $writer.Write($text) } finally { $writer.Dispose() }
}
$zip = New-ZipFile $component
try {
    Add-File $zip $dll 'foo_dj.dll'
    Add-File $zip (Join-Path $repo 'docs/STEM_CACHE.md') 'STEM_CACHE.md'
    Add-File $zip (Join-Path $repo 'docs/PLAYLISTS.md') 'PLAYLISTS.md'
    Add-File $zip (Join-Path $repo 'docs/SCROBBLING_REPLAYGAIN.md') 'SCROBBLING_REPLAYGAIN.md'
    Add-File $zip (Join-Path $repo 'docs/BETA_TEST_PLAN.fr.md') 'BETA_TEST_PLAN.fr.md'
    Add-File $zip (Join-Path $repo 'docs/BETA_RELEASE.fr.md') 'BETA_RELEASE.fr.md'
    Add-File $zip (Join-Path $repo 'docs/HEADPHONE_CUE.md') 'HEADPHONE_CUE.md'
    Add-File $zip (Join-Path $repo 'docs/PLAYLIST_PREVIEW.md') 'PLAYLIST_PREVIEW.md'
    Add-File $zip (Join-Path $repo 'docs/SILENCE_TRIM.md') 'SILENCE_TRIM.md'
    Add-File $zip (Join-Path $repo 'docs/AB_LOOPS.md') 'AB_LOOPS.md'
    Add-File $zip (Join-Path $repo 'docs/SETTINGS.md') 'SETTINGS.md'
    Add-File $zip (Join-Path $repo 'docs/SESSION_FILES.md') 'SESSION_FILES.md'
    Add-File $zip (Join-Path $repo 'docs/TOOLTIPS.md') 'TOOLTIPS.md'
    Add-File $zip (Join-Path $repo 'docs/FABCORE_FIXES.fr.md') 'FABCORE_FIXES.fr.md'
    Add-File $zip (Join-Path $repo 'docs/BPM_ANALYSERS.md') 'BPM_ANALYSERS.md'
    Add-File $zip (Join-Path $repo 'docs/MASTER_LIMITER.fr.md') 'MASTER_LIMITER.fr.md'
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repo 'stems') -File) { Add-File $zip $file.FullName ('stems/'+$file.Name) }
    Add-File $zip (Join-Path $repo 'docs\STEMS.md') 'stems/README.md'
    foreach ($name in @('LICENSE','THIRD_PARTY_NOTICES.md','README.md','README.fr.md','CHANGELOG.md')) { Add-File $zip (Join-Path $repo $name) $name }
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repo 'licenses') -File) { Add-File $zip $file.FullName ('licenses/'+$file.Name) }
    Add-Text $zip 'SOURCE.txt' "Matching complete source: foo_dj-$version-source.zip. Distribute the matching source archive alongside this binary package. Build instructions and all non-system source/build inputs are included in that archive. Beta release. Stem separation remains experimental. See docs/VALIDATION.md in the source archive for architecture-specific checks; real-host feedback remains welcome."
} finally { $zip.Dispose() }
$zip = New-ZipFile $sourceZip
try {
    foreach ($file in Get-ChildItem -LiteralPath $repo -File -Recurse -Force | Sort-Object FullName) {
        $relative = $file.FullName.Substring($repo.Length+1).Replace('\','/')
        if ($relative -match '(^|/)(\.git|\.vs|build|dist|x64|Debug|Release|ARM64|ARM64EC|__pycache__)/') { continue }
        if ($file.Extension -eq '.pyc') { continue }
        if ($file.Name -like '*.dj-decks.json') { continue }
        if ($file.Extension -eq '.json' -and [IO.File]::ReadAllText($file.FullName) -match '"format"\s*:\s*"foo_dj\.session"') { continue }
        if ($file.Extension -match '^\.(obj|pdb|pch|exe|dll|log|user|suo|ilk|idb|exp|tlog|zip|fb2k-component)$') { continue }
        if ($file.Extension -eq '.lib' -and $relative -ne 'sdk/foobar2000/shared/shared-x64.lib') { continue }
        if ($file.Name -in @('foo_dj-session.json','foo_dj-grids.json','foo_dj-loops.json','foo_dj-before-import.json','test-grid-save.json')) { throw 'Local state unexpectedly present in source tree.' }
        Add-File $zip $file.FullName ('foobar-dj-decks/'+$relative)
    }
} finally { $zip.Dispose() }
$engineZip = Join-Path $dist "foo_dj-stems-engine-$version.zip"
$zip = New-ZipFile $engineZip
try {
    foreach ($name in @('install-stems.ps1','stems_worker.py')) { Add-File $zip (Join-Path $repo ('stems/'+$name)) $name }
    Add-File $zip (Join-Path $repo 'docs/STEMS.md') 'README.md'
    foreach ($name in @('LICENSE','THIRD_PARTY_NOTICES.md')) { Add-File $zip (Join-Path $repo $name) $name }
} finally { $zip.Dispose() }
$lines = foreach ($file in @($component,$sourceZip,$engineZip)) { (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()+'  '+[IO.Path]::GetFileName($file) }
[IO.File]::WriteAllLines((Join-Path $dist 'SHA256SUMS-x86.txt'),$lines,[Text.UTF8Encoding]::new($false))
Write-Output "Prepared component, matching source and checksums under dist/. No installation or publication performed."
