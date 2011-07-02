rd /y "lib/vc_amd64_lib"

cd build
cd msw

set options=TARGET_CPU=amd64 SHARED=0 RUNTIME_LIBS=static VENDOR=rsxplusplus UNICODE=1 USE_GUI=1 USE_AUI=1 USE_RTTI=1 USE_EXCEPTIONS=0 USE_RIBBON=0
set options_release=%options% DEBUG_INFO=0 DEBUG_FLAG=0 BUILD=release
set options_debug=%options% DEBUG_INFO=1 DEBUG_FLAG=1 BUILD=debug

nmake -f makefile.vc clean %options_release%
nmake -f makefile.vc %options_release%

nmake -f makefile.vc clean %options_debug%
nmake -f makefile.vc %options_debug%

cd ..
cd ..

xcopy "lib\vc_amd64_lib\*" "lib64\" /y /i
