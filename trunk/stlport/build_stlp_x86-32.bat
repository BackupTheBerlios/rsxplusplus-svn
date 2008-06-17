@echo off
cd build\lib
call "%programfiles%\microsoft visual studio 2005\VC\bin\vcvars32.bat"
call configure.bat -c msvc8
nmake /fmsvc.mak clean all
nmake /fmsvc.mak install