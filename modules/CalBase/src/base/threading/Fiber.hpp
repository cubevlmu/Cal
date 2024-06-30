#pragma once

#ifdef __MACH__
#include <sys/_types/_ucontext.h>
#endif

namespace cal::fiber
{

#ifdef _WIN32
    using Handle = void*;
    using FiberProc = void(__stdcall*)(void*);
    constexpr Handle INVALID_FIBER = nullptr;
#else 
    using Handle = ucontext_t;
    using FiberProc = void (*)(void*);
    constexpr Handle INVALID_FIBER = {};
#endif


    void initThread(FiberProc proc, Handle* handle);
    Handle create(int stack_size, FiberProc proc, void* parameter);
    void destroy(Handle fiber);
    void switchTo(Handle* from, Handle fiber);
    bool isValid(Handle handle);


}