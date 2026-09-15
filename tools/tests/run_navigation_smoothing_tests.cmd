@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist x64\NavigationSmoothingTests mkdir x64\NavigationSmoothingTests
cl /nologo /EHsc /utf-8 /std:c++20 /I. /ISource /IExternal tools\tests\navigation_smoothing_tests.cpp Source\Game\ControllerBehavior\EnemyAI\navigation_system.cpp /Fo:x64\NavigationSmoothingTests\ /Fe:x64\NavigationSmoothingTests\navigation_smoothing_tests.exe
if errorlevel 1 exit /b 1
x64\NavigationSmoothingTests\navigation_smoothing_tests.exe
