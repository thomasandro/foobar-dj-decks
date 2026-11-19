@echo off
setlocal
call "%DJ_DECKS_VCVARS%" >nul
if errorlevel 1 exit /b 1
pushd "%~dp0.."
for %%T in (test-jog-bend test-midi test-midi-routing) do (
 cl /nologo /EHsc /MD /O2 /utf-8 /std:c++20 "tests\%%T.cpp" /Fe:"build\tests\%%T.exe" /Fo:"build\tests\%%T.obj"
 if errorlevel 1 goto failed
 "build\tests\%%T.exe"
 if errorlevel 1 goto failed
)
popd
exit /b 0
:failed
popd
exit /b 1
