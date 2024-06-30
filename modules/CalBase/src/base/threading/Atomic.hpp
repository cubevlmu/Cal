#pragma once

#include "globals.hpp"

namespace cal {

    i64 atomicIncrement(i64 volatile* value);
    i32 atomicIncrement(i32 volatile* value);
    i32 atomicDecrement(i32 volatile* value);

    i32 atomicAdd(i32 volatile* addend, i32 value);
    i64 atomicAdd(i64 volatile* addend, i64 value);
    
    i32 atomicSubtract(i32 volatile* addend, i32 value);
    i64 atomicSubtract(i64 volatile* addend, i64 value);
    
    bool compareAndExchange(i32 volatile* dest, i32 exchange, i32 comperand);
    bool compareAndExchange64(i64 volatile* dest, i64 exchange, i64 comperand);
    void memoryBarrier();
}