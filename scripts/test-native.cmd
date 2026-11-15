@echo off
setlocal EnableExtensions
if not defined DJ_DECKS_VCVARS exit /b 2
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 2
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
for %%T in (test-drop test-tempo test-beats test-key test-skin test-language test-grid-storage test-queue test-queue-transfer test-queue-window test-stems test-stem-cache-manager test-listening-options test-scrobble-detection test-queue-duration test-cue test-preview test-preview-visual test-silence test-loop-storage) do (
  cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 "tests\%%T.cpp" /Fe:"build\tests\%%T.exe" /Fo:"build\tests\%%T.obj"
  if errorlevel 1 goto failed
if not errorlevel 0 goto failed
  "build\tests\%%T.exe"
  if errorlevel 1 goto failed
if not errorlevel 0 goto failed
)
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\preview-generated.cpp /Fe:build\tests\test-preview-decode.exe /Fo:build\tests\test-preview-decode.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-preview-decode.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\preparation-generated.cpp /Fe:build\tests\test-preparation.exe /Fo:build\tests\test-preparation.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-preparation.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\seek-generated.cpp /Fe:build\tests\test-auto-seek.exe /Fo:build\tests\test-auto-seek.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-auto-seek.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\auto-generated.cpp /Fe:build\tests\test-auto.exe /Fo:build\tests\test-auto.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-auto.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\replaygain-generated.cpp /Fe:build\tests\test-replaygain.exe /Fo:build\tests\test-replaygain.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-replaygain.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\scrobble-generated.cpp /Fe:build\tests\test-scrobble.exe /Fo:build\tests\test-scrobble.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-scrobble.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\queue-metadata-generated.cpp /Fe:build\tests\test-queue-metadata.exe /Fo:build\tests\test-queue-metadata.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-queue-metadata.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\queue-label-generated.cpp /Fe:build\tests\test-queue-label.exe /Fo:build\tests\test-queue-label.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-queue-label.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 tests\test-loop.cpp /Fe:build\tests\test-loop.exe /Fo:build\tests\test-loop.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-loop.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\loop-generated.cpp /Fe:build\tests\test-loop-controls.exe /Fo:build\tests\test-loop-controls.obj
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-loop-controls.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
set "ST_OBJECTS="
for %%F in (sdk\foobar2000\foo_dj\third_party\soundtouch\source\*.cpp) do call set "ST_OBJECTS=%%ST_OBJECTS%% build\obj\foo_dj\%%~nF.obj"
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\audio-generated.cpp %ST_OBJECTS% /Fe:build\tests\test-audio.exe /Fo:build\tests\test-audio.obj /link /LTCG
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
build\tests\test-audio.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
if defined DJ_STEM_ENGINE (
  cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 tests\test-stem-job.cpp /Fe:build\tests\test-stem-job.exe /Fo:build\tests\test-stem-job.obj
  if errorlevel 1 goto failed
if not errorlevel 0 goto failed
  build\tests\test-stem-job.exe "%DJ_STEM_ENGINE%"
  if errorlevel 1 goto failed
if not errorlevel 0 goto failed
)
popd
exit /b 0
:failed
popd
exit /b 1
