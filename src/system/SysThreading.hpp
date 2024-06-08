#pragma once

#ifndef _WIN32
#include <pthread.h>
#endif
#include <globals.hpp>

namespace cal::platform {

#ifdef _WIN32
    using ThreadID = u32;
#else
    using ThreadID = pthread_t;
#endif

    u32 getCPUsCount();
    void sleep(u32 milliseconds);
    ThreadID getCurrentThreadID();
}