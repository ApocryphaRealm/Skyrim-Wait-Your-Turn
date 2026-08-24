@echo off
setlocal
REM Build the plugin with xmake (https://xmake.io).
REM
REM Needs xmake on PATH, or set XMAKE to its executable. Visual Studio is located by
REM find-msvc.bat. Clone with --recurse-submodules first: CommonLibSSE-NG is a submodule.

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

if "%XMAKE%"=="" set "XMAKE=xmake"

cd /d "%~dp0"
"%XMAKE%" f -y -m releasedbg -p windows -a x64
if errorlevel 1 exit /b 1
"%XMAKE%" -y
