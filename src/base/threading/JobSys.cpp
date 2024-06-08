#include "JobSys.hpp"

#include "base/Logger.hpp"
#include "base/allocator/TagAllocator.hpp"
#include "base/math/Math.hpp"

#include "base/types/RingBuffer.hpp"
#include "base/threading/Fiber.hpp"
#include "base/types/Array.hpp"
#include "system/SysThreading.hpp"
#include "Thread.hpp"

namespace cal::jobs {

    struct Job {
        void (*task)(void*) = nullptr;
        void* data = nullptr;
        Signal* dec_on_finish;
        u8 worker_index;
    };

    struct WorkerTask;

    struct FiberDecl {
        int idx;
        fiber::Handle fiber = fiber::INVALID_FIBER;
        Job current_job;
    };

#ifdef _WIN32
    static void __stdcall manage(void* data);
#else
    static void manage(void* data);
#endif


    struct Work {
        Work() : type(NONE) {}
        Work(const Job& job) : job(job), type(JOB) {}
        Work(FiberDecl* fiber) : fiber(fiber), type(FIBER) {}
        union {
            Job job;
            FiberDecl* fiber;
        };
        enum Type {
            FIBER,
            JOB,
            NONE
        };
        Type type;
    };


    struct System {
        System(IAllocator& allocator)
            : m_allocator(allocator, "job system")
            , m_workers(m_allocator)
            , m_free_fibers(m_allocator)
            , m_backup_workers(m_allocator)
            , m_work_queue(m_allocator)
            , m_sleeping_workers(m_allocator)
        {}


        TagAllocator m_allocator;
        cal::Mutex m_sync;
        cal::Mutex m_job_queue_sync;
        cal::Mutex m_sleeping_sync;
        Array<WorkerTask*> m_sleeping_workers;
        Array<WorkerTask*> m_workers;
        Array<WorkerTask*> m_backup_workers;
        FiberDecl m_fiber_pool[512];
        Array<FiberDecl*> m_free_fibers;
        RingBuffer<Work, 64> m_work_queue;
    };


    static Local<System> g_system;

    static volatile i32 g_generation = 0;
    static thread_local WorkerTask* g_worker = nullptr;

#ifndef _WIN32
#pragma clang optimize off 
#endif
#pragma optimize( "", off )
    WorkerTask* getWorker()
    {
        return g_worker;
    }
#pragma optimize( "", on )
#ifndef _WIN32
#pragma clang optimize on
#endif


    struct WorkerTask : Thread
    {
        WorkerTask(System& system, u8 worker_index)
            : Thread(system.m_allocator)
            , m_system(system)
            , m_worker_index(worker_index)
            , m_work_queue(system.m_allocator)
        {
        }


        int run() override
        {
            // profiler::showInProfiler(true);
            g_worker = this;
            fiber::initThread(start, &m_primary_fiber);
            return 0;
        }


#ifdef _WIN32
        static void __stdcall start(void* data)
#else
        static void start(void* data)
#endif
        {
            g_system->m_sync.enter();
            FiberDecl* fiber = g_system->m_free_fibers.back();
            g_system->m_free_fibers.pop();
            if (!fiber::isValid(fiber->fiber)) {
                fiber->fiber = fiber::create(64 * 1024, manage, fiber); //TODO CHECK UP
            }
            getWorker()->m_current_fiber = fiber;
            fiber::switchTo(&getWorker()->m_primary_fiber, fiber->fiber);
        }


        bool m_finished = false;
        FiberDecl* m_current_fiber = nullptr;
        fiber::Handle m_primary_fiber;
        System& m_system;
        RingBuffer<Work, 4> m_work_queue;
        u8 m_worker_index;
        bool m_is_enabled = false;
        bool m_is_backup = false;
    };


    struct Waitor {
        Waitor* next;
        FiberDecl* fiber;
    };


    void wake() {
        cal::MutexGuard lock(g_system->m_sleeping_sync);

        for (WorkerTask* task : g_system->m_sleeping_workers) {
            task->wakeup();
        }
        g_system->m_sleeping_workers.clear();
    }


    template <bool ZERO>
    inline static bool trigger(Signal* signal)
    {
        Waitor* waitor = nullptr;
        {
            cal::MutexGuard lock(g_system->m_sync);

            if constexpr (ZERO) {
                signal->counter = 0;
            }
            else {
                --signal->counter;
                ASSERT(signal->counter >= 0);
                if (signal->counter > 0) return false;
            }

            waitor = signal->waitor;
            signal->waitor = nullptr;
        }
        if (!waitor) return false;

        {
            while (waitor) {
                Waitor* next = waitor->next;
                const u8 worker_idx = waitor->fiber->current_job.worker_index;
                if (worker_idx == ANY_WORKER) {
                    g_system->m_work_queue.push(waitor->fiber, &g_system->m_job_queue_sync);
                }
                else {
                    WorkerTask* worker = g_system->m_workers[worker_idx % g_system->m_workers.size()];
                    worker->m_work_queue.push(waitor->fiber, &g_system->m_job_queue_sync);
                }
                waitor = next;
            }
        }

        wake();

        return true;
    }


    void enableBackupWorker(bool enable)
    {
        cal::MutexGuard lock(g_system->m_sync);

        for (WorkerTask* task : g_system->m_backup_workers) {
            if (task->m_is_enabled != enable) {
                task->m_is_enabled = enable;
                return;
            }
        }

        ASSERT(enable);
        WorkerTask* task = CAL_NEW(g_system->m_allocator, WorkerTask)(*g_system, 0xff);
        if (task->create("Backup worker", false)) {
            g_system->m_backup_workers.push(task);
            task->m_is_enabled = true;
            task->m_is_backup = true;
        }
        else {
            LogError("Job system backup worker failed to initialize.");
            CAL_DEL(g_system->m_allocator, task);
        }
    }


    inline static bool setRedEx(Signal* signal) {
        ASSERT(signal);
        ASSERT(signal->counter <= 1);
        bool res = compareAndExchange(&signal->counter, 1, 0);
        if (res) {
            signal->generation = atomicIncrement(&g_generation);
        }
        return res;
    }


    void setRed(Signal* signal) {
        setRedEx(signal);
    }


    void setGreen(Signal* signal) {
        ASSERT(signal);
        ASSERT(signal->counter <= 1);
        const u32 gen = signal->generation;
        // if (trigger<true>(signal)) {
        //     profiler::signalTriggered(gen);
        // }
    }


    void run(void* data, void(*task)(void*), Signal* on_finished)
    {
        runEx(data, task, on_finished, ANY_WORKER);
    }


    void runEx(void* data, void(*task)(void*), Signal* on_finished, u8 worker_index)
    {
        Job job;
        job.data = data;
        job.task = task;
        job.worker_index = worker_index != ANY_WORKER ? worker_index % getWorkersCount() : worker_index;
        job.dec_on_finish = on_finished;

        if (on_finished) {
            cal::MutexGuard guard(g_system->m_sync);
            ++on_finished->counter;
            if (on_finished->counter == 1) {
                on_finished->generation = atomicIncrement(&g_generation);
            }
        }

        if (worker_index != ANY_WORKER) {
            WorkerTask* worker = g_system->m_workers[worker_index % g_system->m_workers.size()];
            worker->m_work_queue.push(job, &g_system->m_job_queue_sync);
            wake();
            return;
        }

        g_system->m_work_queue.push(job, &g_system->m_job_queue_sync);
        wake();
    }


    static bool popWork(Work& work, WorkerTask* worker) {
        if (worker->m_work_queue.pop(work)) return true;
        if (g_system->m_work_queue.pop(work)) return true;

        cal::MutexGuard lock(g_system->m_job_queue_sync);
        if (worker->m_work_queue.popSecondary(work)) return true;
        if (g_system->m_work_queue.popSecondary(work)) return true;

        return false;
    }


#ifdef _WIN32
    static void __stdcall manage(void* data)
#else
    static void manage(void* data)
#endif
    {
        g_system->m_sync.exit();
       
        #ifdef __MACH__
        auto* this_fiber = ((FiberDecl*)((uintptr_t)data + 0x0000000100000000));
        #else 
        FiberDecl* this_fiber = (FiberDecl*)data;
        #endif

        WorkerTask* worker = getWorker();
        while (!worker->m_finished) {
            if (worker->m_is_backup) {
                cal::MutexGuard guard(g_system->m_sync);
                while (!worker->m_is_enabled && !worker->m_finished) {
                    //TODO PROFILE_BLOCK("disabled");
                    // profiler::blockColor(0xff, 0, 0xff);
                    worker->sleep(g_system->m_sync);
                }
            }

            Work work;
            while (!worker->m_finished) {
                if (popWork(work, worker)) break;

                {
                    cal::MutexGuard queue_guard(g_system->m_sleeping_sync);
                    if (popWork(work, worker)) break;

                    // PROFILE_BLOCK("sleeping");
                    // profiler::blockColor(0x30, 0x30, 0x30);
                    g_system->m_sleeping_workers.push(worker);
                    worker->sleep(g_system->m_sleeping_sync);
                }

                if (worker->m_is_backup) break;
            }
            if (worker->m_finished) break;

            if (work.type == Work::FIBER) {
                worker->m_current_fiber = work.fiber;

                g_system->m_sync.enter();
                g_system->m_free_fibers.push(this_fiber);
                fiber::switchTo(&this_fiber->fiber, work.fiber->fiber);
                g_system->m_sync.exit();

                worker = getWorker();
                worker->m_current_fiber = this_fiber;
            }
            else if (work.type == Work::JOB) {
                if (!work.job.task) continue;

                // profiler::beginBlock("job");
                // profiler::blockColor(0x60, 0x60, 0x60);
                // if (work.job.dec_on_finish) {
                //     profiler::pushJobInfo(work.job.dec_on_finish->generation);
                // }
                
                this_fiber->current_job = work.job;
                work.job.task(work.job.data);
                this_fiber->current_job.task = nullptr;
                if (work.job.dec_on_finish) {
                    trigger<false>(work.job.dec_on_finish);
                }
                worker = getWorker();
               // profiler::endBlock();
            }
        }
        fiber::switchTo(&this_fiber->fiber, getWorker()->m_primary_fiber);
    }


    IAllocator& getAllocator() {
        return g_system->m_allocator;
    }
    

    bool init(u8 workers_count, IAllocator& allocator)
    {
#if defined(__MACH__) &&  defined (TOY_DEBUG)
        LogDebug("[JobSys] We Thank Apple That Make Our Work More Complex...");
        LogDebug("[JobSys] This Message Only Display When Framework In Debug Mode");
        LogDebug("[JobSys] So F**k Apple");
#endif
        g_system.create(allocator);

        g_system->m_free_fibers.reserve(sizeof(g_system->m_fiber_pool));
        for (FiberDecl& fiber : g_system->m_fiber_pool) {
            g_system->m_free_fibers.push(&fiber);
        }

        const int fiber_num = sizeof(g_system->m_fiber_pool);
        for (int i = 0; i < fiber_num; ++i) {
            FiberDecl& decl = g_system->m_fiber_pool[i];
            decl.idx = i;
        }

        int count = maximum(1, int(workers_count));
        for (int i = 0; i < count; ++i) {
            WorkerTask* task = CAL_NEW(getAllocator(), WorkerTask)(*g_system, i < 64 ? u64(1) << i : 0);
            if (task->create("Worker", false)) {
                task->m_is_enabled = true;
                g_system->m_workers.push(task);
                task->setAffinityMask((u64)1 << i);
            }
            else {
                LogError("Job system worker failed to initialize.");
                CAL_DEL(getAllocator(), task);
            }
        }

        return !g_system->m_workers.empty();
    }


    u8 getWorkersCount()
    {
        const int c = g_system->m_workers.size();
        ASSERT(c <= 0xff);
        return (u8)c;
    }


    void shutdown()
    {
        IAllocator& allocator = g_system->m_allocator;
        for (Thread* task : g_system->m_workers)
        {
            WorkerTask* wt = (WorkerTask*)task;
            wt->m_finished = true;
        }
        for (Thread* task : g_system->m_backup_workers) {
            WorkerTask* wt = (WorkerTask*)task;
            wt->m_finished = true;
            wt->wakeup();
        }

        for (Thread* task : g_system->m_backup_workers)
        {
            while (!task->isFinished()) task->wakeup();
            task->destroy();
            CAL_DEL(allocator, task);
        }

        for (WorkerTask* task : g_system->m_workers)
        {
            while (!task->isFinished()) task->wakeup();
            task->destroy();
            CAL_DEL(allocator, task);
        }

        for (FiberDecl& fiber : g_system->m_fiber_pool)
        {
            if (fiber::isValid(fiber.fiber)) {
                fiber::destroy(fiber.fiber);
            }
        }

        g_system.destroy();
    }


    static void waitEx(Signal* signal, bool is_mutex)
    {
        ASSERT(signal);
        g_system->m_sync.enter();

        if (signal->counter == 0) {
            g_system->m_sync.exit();
            return;
        }

        if (!getWorker()) {
            while (signal->counter > 0) {
                g_system->m_sync.exit();
                cal::platform::sleep(1);
                g_system->m_sync.enter();
            }
            g_system->m_sync.exit();
            return;
        }

        FiberDecl* this_fiber = getWorker()->m_current_fiber;

        Waitor waitor;
        waitor.fiber = this_fiber;
        waitor.next = signal->waitor;
        signal->waitor = &waitor;

        //const profiler::FiberSwitchData& switch_data = profiler::beginFiberWait(signal->generation, is_mutex);
        FiberDecl* new_fiber = g_system->m_free_fibers.back();
        g_system->m_free_fibers.pop();
        if (!fiber::isValid(new_fiber->fiber)) {
            new_fiber->fiber = fiber::create(64 * 1024, manage, new_fiber);
        }
        getWorker()->m_current_fiber = new_fiber;
        fiber::switchTo(&this_fiber->fiber, new_fiber->fiber);
        getWorker()->m_current_fiber = this_fiber;
        g_system->m_sync.exit();
        // profiler::endFiberWait(switch_data);
    }


    void enter(Mutex* mutex) {
        ASSERT(getWorker());
        for (;;) {
            for (u32 i = 0; i < 400; ++i) {
                if (setRedEx(&mutex->signal)) return;
            }
            waitEx(&mutex->signal, true);
        }
    }


    void exit(Mutex* mutex) {
        ASSERT(getWorker());
        setGreen(&mutex->signal);
    }
    

    void wait(Signal* handle) {
        waitEx(handle, false);
    }

}