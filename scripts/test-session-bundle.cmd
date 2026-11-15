@echo off
setlocal
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\session-generated.cpp /Fe:build\tests\test-session-bundle.exe /Fo:build\tests\test-session-bundle.obj
if errorlevel 1 goto failed
build\tests\test-session-bundle.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 tests\test-session-file.cpp /Fe:build\tests\test-session-file.exe /Fo:build\tests\test-session-file.obj
if errorlevel 1 goto failed
build\tests\test-session-file.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\preparation-generated.cpp /Fe:build\tests\test-preparation.exe /Fo:build\tests\test-preparation.obj
if errorlevel 1 goto failed
build\tests\test-preparation.exe
if errorlevel 1 goto failed
if not errorlevel 0 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
