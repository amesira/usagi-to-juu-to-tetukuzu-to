@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /utf-8 /std:c++17 /I. /ISource tools\tests\player_ui_presentation_tests.cpp Source\Game\PresBehavior\UI\Player\player_ui_presentation.cpp Source\Game\PresBehavior\UI\Player\player_ui_view.cpp /Fo:x64\Debug\ /Fe:x64\Debug\player_ui_presentation_tests.exe
if errorlevel 1 exit /b 1
x64\Debug\player_ui_presentation_tests.exe