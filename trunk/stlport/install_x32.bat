@echo off

del /Q lib\*.*

call configure.bat msvc8
cd build\lib
call "W:\Program Files (x86)\microsoft visual studio 8\VC\bin\vcvars32.bat"

nmake /fmsvc.mak clean all
nmake /fmsvc.mak install

cd ../..
xcopy /Y /C lib\stlport.5.2.exp lib32\
xcopy /Y /C lib\stlport.5.2.lib lib32\
xcopy /Y /C lib\stlport_static.lib lib32\
xcopy /Y /C lib\stlport_static.pdb lib32\
xcopy /Y /C lib\stlportd.5.2.exp lib32\
xcopy /Y /C lib\stlportd.5.2.lib lib32\
xcopy /Y /C lib\stlportd_static.lib lib32\
xcopy /Y /C lib\stlportd_static.pdb lib32\
xcopy /Y /C lib\stlportstld.5.2.exp lib32\
xcopy /Y /C lib\stlportstld.5.2.lib lib32\
xcopy /Y /C lib\stlportstld_static.lib lib32\
xcopy /Y /C lib\stlportstld_static.pdb lib32\

del /Q lib\*.*