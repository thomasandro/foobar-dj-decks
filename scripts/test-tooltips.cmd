@echo off
setlocal
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\tooltip-generated.cpp /Fe:build\tests\test-tooltips.exe /Fo:build\tests\test-tooltips.obj
if errorlevel 1 goto failed
build\tests\test-tooltips.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
if defined DJ_HOVER_BENCH (
 cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 tests\test-hover-paint.cpp /Fe:build\tests\test-hover-paint.exe /Fo:build\tests\test-hover-paint.obj
 if errorlevel 1 goto failed
 build\tests\test-hover-paint.exe
 if errorlevel 1 goto failed
 if not errorlevel 0 goto failed
)
popd
exit /b 0
:failed
popd
exit /b 1
