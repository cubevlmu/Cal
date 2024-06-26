#ifdef __MACH__

#include "base/threading/Atomic.hpp"

namespace cal {

    i32 atomicIncrement(i32 volatile* value)
    {
        return __sync_fetch_and_add(value, 1) + 1;
    }

    i64 atomicIncrement(i64 volatile* value)
    {
        return __sync_fetch_and_add(value, 1) + 1;
    }

    i32 atomicDecrement(i32 volatile* value)
    {
        return __sync_fetch_and_sub(value, 1) - 1;
    }

    i32 atomicAdd(i32 volatile* addend, i32 value)
    {
        return __sync_fetch_and_add(addend, value);
    }

    i32 atomicSubtract(i32 volatile* addend, i32 value)
    {
        return __sync_fetch_and_sub(addend, value);
    }

    i64 atomicAdd(i64 volatile* addend, i64 value)
    {
        return __sync_fetch_and_add(addend, value);
    }

    i64 atomicSubtract(i64 volatile* addend, i64 value)
    {
        return __sync_fetch_and_sub(addend, value);
    }

    bool compareAndExchange(i32 volatile* dest, i32 exchange, i32 comperand)
    {
        return __sync_bool_compare_and_swap(dest, comperand, exchange);
    }

    bool compareAndExchange64(i64 volatile* dest, i64 exchange, i64 comperand)
    {
        return __sync_bool_compare_and_swap(dest, comperand, exchange);
    }


    void memoryBarrier()
    {
        __sync_synchronize();
    }

}

#endif