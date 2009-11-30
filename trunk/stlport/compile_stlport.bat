@echo off

call configure.bat msvc9 --with-lib-motif 32
cd build
cd lib

call "C:\Program Files (x86)\microsoft visual studio 9.0\VC\bin\vcvars32.bat"
nmake clean install

cd ..
cd ..

call configure.bat msvc8 --with-lib-motif 64
cd build
cd lib

call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
nmake clean install

cd ..
cd ..