@echo off
setlocal

REM run premake
call vendor\premake5\premake5.exe vs2022

endlocal
pause