#pragma once

#include "base/threading/Atomic.hpp"
#include "Array.hpp"
#include "base/threading/SyncMutex.hpp"

namespace cal {

    template <typename T, u32 CAPACITY>
    struct RingBuffer {
        struct Item {
            T value;
            volatile i32 seq;
        };

        RingBuffer(IAllocator& allocator)
            : m_fallback(allocator)
        {
            static_assert(CAPACITY > 2);
            for (u32 i = 0; i < CAPACITY; ++i) {
                objects[i].seq = i;
            }
            memoryBarrier();
        }

        inline bool pop(T& obj) {
            i32 pos = rd;
            Item* j;
            for (;;) {
                j = &objects[pos % CAPACITY];
                const i32 seq = j->seq;
                if (seq < pos + 1) {
                    return false;
                }
                else if (seq == pos + 1) {
                    if (compareAndExchange(&rd, pos + 1, pos)) break;
                }
                else {
                    pos = rd;
                }
            }
            obj = j->value;
            j->seq = pos + CAPACITY;
            return true;
        }

        inline void push(const T& obj, cal::Mutex* mutex) {
            volatile i32 pos = wr;
            Item* j;
            for (;;) {
                j = &objects[pos % CAPACITY];
                const i32 seq = j->seq;
                if (seq < pos) {
                    // buffer full
                    if (mutex) mutex->enter();
                    m_fallback.push(obj);
                    if (mutex) mutex->exit();
                    return;
                }
                else if (seq == pos) {
                    // we can try to push
                    if (compareAndExchange(&wr, pos + 1, pos)) break;
                }
                else {
                    // somebody pushed before us, try again
                    pos = wr;
                }
            }
            j->value = obj;
            j->seq = pos + 1;
        }

        inline bool popSecondary(T& obj) {
            if (m_fallback.empty()) return false;
            obj = m_fallback.back();
            m_fallback.pop();
            return true;
        }

        Item objects[CAPACITY];
        volatile i32 rd = 0;
        volatile i32 wr = 0;
        Array<T> m_fallback;
    };

}