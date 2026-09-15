@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /utf-8 /std:c++17 /I. /ISource tools\tests\image_fill_tests.cpp /Fo:x64\Debug\image_fill_tests.obj /Fe:x64\Debug\image_fill_tests.exe
if errorlevel 1 exit /b 1
x64\Debug\image_fill_tests.exe