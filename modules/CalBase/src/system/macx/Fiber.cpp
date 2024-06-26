#ifdef __MACH__

#include "globals.hpp"
#include <alloca.h>
#include <malloc/_malloc.h>

#undef __API_DEPRECATED
#define __API_DEPRECATED(...)
#define _XOPEN_SOURCE
#include <ucontext.h>
#undef __API_DEPRECATED

#include "base/threading/Fiber.hpp"

namespace cal::fiber {


    thread_local Handle g_finisher;


    void initThread(FiberProc proc, Handle* out)
    {
        *out = create(64 * 1024, proc, nullptr);

        getcontext(&g_finisher);
        out->uc_link = &g_finisher;
        
        switchTo(&g_finisher, *out);
    }


    Handle create(int stack_size, FiberProc proc, void* parameter)
    {
        ucontext_t fib;
        getcontext(&fib);
        fib.uc_stack.ss_sp = (::malloc)(stack_size);// alloca(stack_size);//
        fib.uc_stack.ss_size = stack_size;
        fib.uc_link = 0;
        makecontext(&fib, reinterpret_cast<void(*)(void)>(proc), 1, &parameter);
        return fib;
    }
    

    bool isValid(Handle handle)
    {
        return handle.uc_stack.ss_sp != nullptr;
    }


    void destroy(Handle fiber)
    {
        ASSERT(false);
    }


    void switchTo(Handle* prev, Handle fiber)
    {
        // profiler::beforeFiberSwitch();
        if (fiber.uc_onstack < 0) return;
        swapcontext(prev, &fiber);
    }


    void* getParameter()
    {
        ASSERT(false);
        return nullptr;
    }

}

#endif