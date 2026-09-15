@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist x64\Debug\audio_tests mkdir x64\Debug\audio_tests
cl /nologo /EHsc /utf-8 /std:c++20 /I. /ISource /IExternal tools\tests\audio_tests.cpp Source\Engine\Device\audio.cpp /Fo:x64\Debug\audio_tests\ /Fe:x64\Debug\audio_tests.exe
if errorlevel 1 exit /b 1
x64\Debug\audio_tests.exe
