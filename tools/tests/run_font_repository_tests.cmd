@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /utf-8 /std:c++17 /I. tools\tests\font_repository_tests.cpp Source\Engine\Graphics\font_repository.cpp /Fo:x64\Debug\ /Fe:x64\Debug\font_repository_tests.exe /link d3d11.lib
if errorlevel 1 exit /b 1
x64\Debug\font_repository_tests.exe