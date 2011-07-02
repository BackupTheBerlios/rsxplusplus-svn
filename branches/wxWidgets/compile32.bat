cd build
cd msw

nmake -f makefile.vc BUILD=release SHARED=0 RUNTIME_LIBS=dynamic DEBUG_INFO=0 VENDOR=rsxplusplus UNICODE=1 USE_GUI=1 USE_AUI=1 USE_RTTI=1 USE_EXCEPTIONS=1 USE_STC=1

cd ..
cd ..

xcopy "lib\vc_lib\*" "lib32\" /y /i
