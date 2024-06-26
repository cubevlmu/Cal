#pragma once

#include "globals.hpp"
#ifndef __WIN32
#include <pthread.h>
#endif

namespace cal {

    struct Semaphore {
        Semaphore(int init_count, int max_count);
        Semaphore(const Semaphore&) = delete;
        ~Semaphore();

        void signal();
        void wait();

    private:
#if defined _WIN32
        void* m_id;
#else
        struct {
            pthread_mutex_t mutex;
            pthread_cond_t cond;
            volatile i32 count;
        } m_id;
#endif
    };

}