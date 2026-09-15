@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /utf-8 /std:c++20 /ISource tools\tests\scene_transition_tests.cpp Source\Engine\Core\scene_transition.cpp /Fo:x64\Debug\ /Fe:x64\Debug\scene_transition_tests.exe
if errorlevel 1 exit /b 1
x64\Debug\scene_transition_tests.exe
