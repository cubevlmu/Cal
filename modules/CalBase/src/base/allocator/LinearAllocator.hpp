#pragma once

#include "IAllocator.hpp"
#include "base/threading/SyncMutex.hpp"
#include "globals.hpp"

namespace cal {
    
    // allocations in a row one after another, deallocate everything at once
    // use case: data for one frame
    struct LinearAllocator : IAllocator {
        LinearAllocator(u32 reserved);
        ~LinearAllocator();

        void reset();
        void* allocate(size_t size, size_t align) override;
        void deallocate(void* ptr) override;
        void* reallocate(void* ptr, size_t new_size, size_t old_size, size_t align) override;

        u32 getCommitedBytes() const { return m_commited_bytes; }
        static size_t getTotalCommitedBytes() { return g_total_commited_bytes; }

    private:
        u32 m_commited_bytes;
        u32 m_reserved;
        volatile i32 m_end;
        u8* m_mem;
        Mutex m_mutex;

        static volatile i64 g_total_commited_bytes;
    };
}