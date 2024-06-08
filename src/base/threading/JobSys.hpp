#pragma once

#include "globals.hpp"
#include "base/allocator/IAllocator.hpp"
#include "Atomic.hpp"

namespace cal::jobs
{
    constexpr u8 ANY_WORKER = 0xff;

    struct Mutex;
    struct Signal;

    bool init(u8 workers_count, IAllocator& allocator);
    IAllocator& getAllocator();
    void shutdown();
    u8 getWorkersCount();

    void enableBackupWorker(bool enable);

    void enter(Mutex* mutex);
    void exit(Mutex* mutex);

    void setRed(Signal* signal);
    void setGreen(Signal* signal);

    void run(void* data, void(*task)(void*), Signal* on_finish);
    void runEx(void* data, void (*task)(void*), Signal* on_finish, u8 worker_index);
    void wait(Signal* signal);


    template <typename Fa>
    void runLambda(Fa&& f, Signal* on_finish, u8 worker = ANY_WORKER) {
        void* arg;
        if constexpr (sizeof(f) == sizeof(void*) && __is_trivially_copyable(Fa)) {
            memcpy(&arg, &f, sizeof(arg));
            runEx(arg, [](void* arg) {
                Fa* f = (Fa*)&arg;
                (*f)();
                }, on_finish, worker);
        }
        else {
            Fa* tmp = CAL_NEW(getAllocator(), Fa)(static_cast<Fa&&>(f));
            runEx(tmp, [](void* arg) {
                Fa* f = (Fa*)arg;
                (*f)();
                TOY_DEL(getAllocator(), f);
                }, on_finish, worker);

        }
    }

    struct MutexGuard {
        MutexGuard(Mutex& mutex) : mutex(mutex) { enter(&mutex); }
        ~MutexGuard() { exit(&mutex); }

        Mutex& mutex;
    };

    struct Signal {
        ~Signal() { ASSERT(!waitor); ASSERT(!counter); }

        struct Waitor* waitor = nullptr;
        volatile i32 counter = 0;
        i32 generation; // identify different red-green pairs on the same signal, used by profiler
    };

    struct Mutex {
        Signal signal; // do not access this outside of job_system.cpp
    };

    template <typename Fa>
    void runOnWorkers(const Fa& f)
    {
        Signal signal;
        for (int i = 1, c = getWorkersCount(); i < c; ++i) {
            jobs::run((void*)&f, [](void* data) {
                (*(const Fa*)data)();
                }, &signal);
        }
        f();
        wait(&signal);
    }


    template <typename Fa>
    void forEach(i32 count, i32 step, const Fa& f)
    {
        if (count == 0) return;
        if (count <= step) {
            f(0, count);
            return;
        }

        volatile i32 offset = 0;

        jobs::runOnWorkers([&]() {
            for (;;) {
                const i32 idx = atomicAdd(&offset, step);
                if (idx >= count) break;
                i32 to = idx + step;
                to = to > count ? count : to;
                f(idx, to);
            }
            });
    }
}