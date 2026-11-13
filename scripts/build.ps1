param([string]$MSBuildPath = '', [string]$VSWHERE = '')
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $MSBuildPath) {
    if (-not $VSWHERE) { $VSWHERE = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe' }
    if (-not (Test-Path -LiteralPath $VSWHERE)) { throw 'vswhere not found; supply -MSBuildPath or -VSWHERE.' }
    $MSBuildPath = & $VSWHERE -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
}
if (-not $MSBuildPath -or -not (Test-Path -LiteralPath $MSBuildPath)) { throw 'MSBuild not found.' }
$build = Join-Path $repo 'build'
New-Item -ItemType Directory -Path $build -Force | Out-Null
$log = Join-Path $build 'build.log'
& $MSBuildPath (Join-Path $repo 'sdk\foobar2000\foo_dj\foo_dj.sln') /p:Configuration=Release /p:Platform=x64 /m /v:minimal /nologo *> $log
if ($LASTEXITCODE -ne 0) { Get-Content -LiteralPath $log -Tail 40; throw 'Build failed; see build/build.log.' }
if (-not (Test-Path -LiteralPath (Join-Path $repo 'build\bin\foo_dj.dll'))) { throw 'Build did not produce foo_dj.dll.' }
Write-Output 'Release x64 build passed: build/bin/foo_dj.dll'
