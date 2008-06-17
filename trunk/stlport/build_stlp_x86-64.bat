@echo off
cd build\lib
call "%programfiles%\Microsoft Visual Studio 2005\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
call configure.bat -c msvc8
nmake /fmsvc.mak clean all
nmake /fmsvc.mak install