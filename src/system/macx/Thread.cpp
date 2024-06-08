#ifdef __MACH__

#include "base/allocator/IAllocator.hpp"
#include "base/threading/Thread.hpp"
#include "base/threading/Sync.hpp"

#include <pthread.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <sched.h>

namespace cal {

    struct ThreadImpl
    {
        IAllocator& allocator;
        bool force_exit;
        bool exited;
        bool is_running;
        pthread_t handle;
        const char* thread_name;
        Thread* owner;
        ConditionVariable cv;
    };

    static void* threadFunction(void* ptr)
    {
        struct ThreadImpl* impl = reinterpret_cast<ThreadImpl*>(ptr);
        pthread_setname_np(impl->thread_name);
        //TODO profiler::setThreadName(impl->thread_name);
        u32 ret = 0xffffFFFF;
        if (!impl->force_exit) ret = impl->owner->run();
        impl->exited = true;
        impl->is_running = false;

        return nullptr;
    }

    Thread::Thread(IAllocator& allocator)
    {
        auto impl = CAL_NEW(allocator, ThreadImpl) { allocator };

        impl->is_running = false;
        impl->force_exit = false;
        impl->exited = false;
        impl->thread_name = "";
        impl->owner = this;

        m_implementation = impl;
    }


    Thread::~Thread()
    {
        CAL_DEL(m_implementation->allocator, m_implementation);
    }


    void Thread::sleep(Mutex& mutex) {
        ASSERT(pthread_self() == m_implementation->handle);
        m_implementation->cv.sleep(mutex);
    }


    void Thread::wakeup() { m_implementation->cv.wakeup(); }


    bool Thread::create(const char* name, bool is_extended)
    {
        pthread_attr_t attr;
        int res = pthread_attr_init(&attr);
        ASSERT(res == 0);
        if (res != 0) return false;
        res = pthread_create(&m_implementation->handle, &attr, threadFunction, m_implementation);
        ASSERT(res == 0);
        if (res != 0) return false;
        return true;
    }


    bool Thread::destroy()
    {
        return pthread_join(m_implementation->handle, nullptr) == 0;
    }


    void Thread::setAffinityMask(u64 affinity_mask)
    {
        thread_affinity_policy_data_t policy;
        policy.affinity_tag = (integer_t)affinity_mask;

        thread_port_t threadport = pthread_mach_thread_np(m_implementation->handle);

        thread_policy_set(threadport, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);

        // cpu_set_t set;
        // CPU_ZERO(&set);
        // for (int i = 0; i < 64; ++i)
        // {
        //     if (affinity_mask & ((u64)1 << i))
        //     {
        //         CPU_SET(i, &set);
        //     }
        // }
        // pthread_setaffinity_np(m_implementation->handle, sizeof(set), &set);
    }

    bool Thread::isRunning() const
    {
        return m_implementation->is_running;
    }

    bool Thread::isFinished() const
    {
        return m_implementation->exited;
    }

    IAllocator& Thread::getAllocator()
    {
        return m_implementation->allocator;
    }

}

#endif