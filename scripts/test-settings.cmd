@echo off
setlocal
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\settings-generated.cpp /Fe:build\tests\test-settings.exe /Fo:build\tests\test-settings.obj
if errorlevel 1 goto failed
build\tests\test-settings.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
