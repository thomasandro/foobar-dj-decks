param([string]$Root = (Join-Path $env:LOCALAPPDATA 'DJDecks\stems-engine'), [switch]$DownloadModel, [switch]$CPUOnly)
$ErrorActionPreference='Stop'
New-Item -ItemType Directory -Path $Root -Force | Out-Null
$python=Join-Path $Root 'venv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $python)) {
    & python -m venv (Join-Path $Root 'venv')
    if ($LASTEXITCODE -ne 0) {throw 'Python 3.10–3.12 is required.'}
}
$index = 'https://download.pytorch.org/whl/cpu'
if (-not $CPUOnly -and (Get-Command nvidia-smi -ErrorAction SilentlyContinue)) {
    & nvidia-smi --query-gpu=name --format=csv,noheader *> $null
    if ($LASTEXITCODE -eq 0) { $index = 'https://download.pytorch.org/whl/cu118' }
}
& $python -m pip install --no-cache-dir --upgrade --force-reinstall --no-deps torch==2.5.1 torchaudio==2.5.1 --index-url $index
if ($LASTEXITCODE -ne 0 -and $index.EndsWith('/cu118')) {
    Write-Warning 'CUDA package installation failed; installing the CPU engine.'
    & $python -m pip install --no-cache-dir --force-reinstall --no-deps torch==2.5.1 torchaudio==2.5.1 --index-url https://download.pytorch.org/whl/cpu
}
if ($LASTEXITCODE -ne 0) {throw 'PyTorch installation failed.'}
& $python -m pip install --no-cache-dir numpy==1.26.4 soundfile==0.13.1 demucs==4.0.1 'setuptools<81'
if ($LASTEXITCODE -ne 0) {throw 'Demucs installation failed.'}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'stems_worker.py') -Destination (Join-Path $Root 'stems_worker.py') -Force
if ($DownloadModel) {
    & $python (Join-Path $Root 'stems_worker.py') --download-model
    if ($LASTEXITCODE -ne 0) {throw 'Model download failed.'}
}
Write-Output "Stem engine ready under $Root. Foobar was not accessed."
