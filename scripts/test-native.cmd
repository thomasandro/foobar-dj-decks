@echo off
setlocal EnableExtensions
if not defined DJ_DECKS_VCVARS exit /b 2
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 2
pushd "%~dp0.."
if not exist build\tests mkdir build\tests
for %%T in (test-drop test-tempo test-beats test-key test-skin test-language test-grid-storage test-queue test-queue-window) do (
  cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 "tests\%%T.cpp" /Fe:"build\tests\%%T.exe" /Fo:"build\tests\%%T.obj"
  if errorlevel 1 goto failed
  "build\tests\%%T.exe"
  if errorlevel 1 goto failed
)
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\auto-generated.cpp /Fe:build\tests\test-auto.exe /Fo:build\tests\test-auto.obj
if errorlevel 1 goto failed
build\tests\test-auto.exe
if errorlevel 1 goto failed
set "ST_OBJECTS="
for %%F in (sdk\foobar2000\foo_dj\third_party\soundtouch\source\*.cpp) do call set "ST_OBJECTS=%%ST_OBJECTS%% build\obj\foo_dj\%%~nF.obj"
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\audio-generated.cpp %ST_OBJECTS% /Fe:build\tests\test-audio.exe /Fo:build\tests\test-audio.obj /link /LTCG
if errorlevel 1 goto failed
build\tests\test-audio.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
