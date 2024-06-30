#pragma once

#ifndef __WIN32
#include <pthread.h>
#endif

namespace cal {

    struct alignas(8) Mutex {
        friend struct ConditionVariable;

        Mutex();
        Mutex(const Mutex&) = delete;
        ~Mutex();

        void enter();
        void exit();

    private:
#ifdef _WIN32
        u8 data[8];
#else
        pthread_mutex_t mutex;
#endif
    };


    struct MutexGuard {
        explicit MutexGuard(Mutex& cs)
            : m_mutex(cs)
        {
            cs.enter();
        }
        ~MutexGuard() { m_mutex.exit(); }

        MutexGuard(const MutexGuard&) = delete;
        void operator=(const MutexGuard&) = delete;

    private:
        Mutex& m_mutex;
    };
}