in compile_stlport.bat change path to your visual studio directory.

in stlport/stl/config/user_config.h add following code BEFORE compiling (doesn't matter where)

#ifdef _WIN64

#define _STLP_LIB_NAME_MOTIF "64"

#else

#define _STLP_LIB_NAME_MOTIF "32"

#endif

then run the script!

have fun ;)