@echo off

call configure.bat msvc8 --with-lib-motif 32
cd build
cd lib

call "W:\Program Files (x86)\microsoft visual studio 8\VC\bin\vcvars32.bat"
nmake clean install

cd ..
cd ..

call configure.bat msvc8 --with-lib-motif 64
cd build
cd lib

call "w:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
nmake clean install

cd ..
cd ..