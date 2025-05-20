#if defined(OS_LINUX)
    #include "linux/os.h"
//    #include "linux/gfx.h"
#elif defined(OS_WINDOWS)
    #error Windows os layer isnt implemented
#elif defined(OS_MAC)
    #error Macos os layer isnt implemented
#endif
