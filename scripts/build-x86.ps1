param([string]$MSBuildPath = '', [string]$VSWHERE = '')
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
if(-not $MSBuildPath){
    if(-not $VSWHERE){$VSWHERE=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'}
    $MSBuildPath=& $VSWHERE -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'MSBuild/**/Bin/MSBuild.exe' | Select-Object -First 1
}
if(-not $MSBuildPath){throw 'Visual Studio C++ tools not found'}
New-Item -ItemType Directory (Join-Path $repo 'build') -Force | Out-Null
# Generate the x86 import library from bundled SDK source; shared.dll is host-provided.
& $MSBuildPath (Join-Path $repo 'sdk/foobar2000/shared/shared.vcxproj') /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v145 /m /v:minimal /nologo *> (Join-Path $repo 'build/build-shared-x86.log')
if($LASTEXITCODE -ne 0){throw 'x86 SDK import library build failed; see build/build-shared-x86.log'}
& $MSBuildPath (Join-Path $repo 'sdk/foobar2000/foo_dj/foo_dj.sln') /p:Configuration=Release /p:Platform=x86 /p:PlatformToolset=v145 /m /v:minimal /nologo *> (Join-Path $repo 'build/build-x86.log')
if($LASTEXITCODE -ne 0){Get-Content (Join-Path $repo 'build/build-x86.log') -Tail 25;throw 'x86 build failed'}
$bytes=[IO.File]::ReadAllBytes((Join-Path $repo 'build/bin/foo_dj.dll'))
$pe=[BitConverter]::ToInt32($bytes,0x3c)
if([BitConverter]::ToUInt16($bytes,$pe+4) -ne 0x14c){throw 'Expected x86 PE binary'}
Write-Output 'Release x86 build passed: build/bin/foo_dj.dll. Do not mix build objects with x64; use a separate source directory.'
