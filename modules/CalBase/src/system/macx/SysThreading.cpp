#ifdef __MACH__

#include <unistd.h>
#include "system/SysThreading.hpp"

namespace cal::platform {

    u32 getCPUsCount() {
        return sysconf(_SC_NPROCESSORS_ONLN);
    }

    void sleep(u32 milliseconds) {
        if (milliseconds) usleep(useconds_t(milliseconds * 1000));
    }
    
    ThreadID getCurrentThreadID() {
        return pthread_self();
    }
}

#endif