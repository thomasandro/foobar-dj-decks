@echo off
setlocal
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
for %%T in (test-loop-storage test-loop test-beats test-skin) do (
 cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 tests\%%T.cpp /Fe:build\tests\%%T.exe /Fo:build\tests\%%T.obj
 if errorlevel 1 goto failed
 build\tests\%%T.exe
 if errorlevel 1 goto failed
 if not errorlevel 0 goto failed
)
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\loop-generated.cpp /Fe:build\tests\test-loop-controls.exe /Fo:build\tests\test-loop-controls.obj
if errorlevel 1 goto failed
build\tests\test-loop-controls.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
