@echo off

del /Q lib\*.*

cd build
call "w:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
call configure.bat -c msvc8
cd lib
nmake /fmsvc.mak clean all
nmake /fmsvc.mak install

cd ../..
xcopy /Y /C lib\stlport.5.2.exp lib64\
xcopy /Y /C lib\stlport.5.2.lib lib64\
xcopy /Y /C lib\stlport_static.lib lib64\
xcopy /Y /C lib\stlport_static.pdb lib64\
xcopy /Y /C lib\stlportd.5.2.exp lib64\
xcopy /Y /C lib\stlportd.5.2.lib lib64\
xcopy /Y /C lib\stlportd_static.lib lib64\
xcopy /Y /C lib\stlportd_static.pdb lib64\
xcopy /Y /C lib\stlportstld.5.2.exp lib64\
xcopy /Y /C lib\stlportstld.5.2.lib lib64\
xcopy /Y /C lib\stlportstld_static.lib lib64\
xcopy /Y /C lib\stlportstld_static.pdb lib64\

del /Q lib\*.*