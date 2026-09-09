@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /utf-8 /std:c++20 /I. /ISource /IExternal tools\tests\navigation_system_tests.cpp Source\Game\ControllerBehavior\EnemyAI\navigation_system.cpp /Fo:x64\Debug\ /Fe:x64\Debug\navigation_system_tests.exe
if errorlevel 1 exit /b 1
x64\Debug\navigation_system_tests.exe
