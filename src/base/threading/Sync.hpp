#pragma once

#include "SyncMutex.hpp"

namespace cal {

    struct ConditionVariable {
        ConditionVariable();
        ConditionVariable(const ConditionVariable&) = delete;
        ~ConditionVariable();

        void sleep(Mutex& cs);
        void wakeup();
    private:
#ifdef _WIN32
        u8 data[64];
#else
        pthread_cond_t cv;
#endif
    };
}