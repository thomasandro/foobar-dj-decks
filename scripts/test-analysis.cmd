@echo off
setlocal
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 build\tests\analysis-queue-generated.cpp /Fe:build\tests\test-analysis-queue.exe /Fo:build\tests\test-analysis-queue.obj
if errorlevel 1 exit /b 1
build\tests\test-analysis-queue.exe
exit /b %errorlevel%
