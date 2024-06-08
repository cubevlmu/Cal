#include "LinearAllocator.hpp"

#include "base/threading/Atomic.hpp"
#include "system/Sys.hpp"
#include "base/math/Math.hpp"

namespace cal {

    LinearAllocator::LinearAllocator(u32 reserved) {
        m_end = 0;
        m_commited_bytes = 0;
        m_reserved = reserved;
        m_mem = (u8*)platform::memReserve(reserved);
    }

    LinearAllocator::~LinearAllocator() {
        ASSERT(m_end == 0);
        platform::memRelease(m_mem, m_reserved);
        atomicSubtract(&g_total_commited_bytes, m_commited_bytes);
    }

    void LinearAllocator::reset() {
        m_end = 0;
    }

    static u32 roundUp(u32 val, u32 align) {
        ASSERT(isPowOfTwo(align));
        return (val + align - 1) & ~(align - 1);
    }

    void* LinearAllocator::allocate(size_t size, size_t align) {
        ASSERT(size < 0xffFFffFF);
        u32 start;
        for (;;) {
            const u32 end = m_end;
            start = roundUp(end, (u32)align);
            if (compareAndExchange(&m_end, u32(start + size), end)) break;
        }

        if (start + size <= m_commited_bytes) return m_mem + start;

        MutexGuard guard(m_mutex);
        if (start + size <= m_commited_bytes) return m_mem + start;

        const u32 commited = roundUp(start + (u32)size, 4096);
        ASSERT(commited < m_reserved);
        platform::memCommit(m_mem + m_commited_bytes, commited - m_commited_bytes);
        atomicAdd(&g_total_commited_bytes, commited - m_commited_bytes);
        m_commited_bytes = commited;

        return m_mem + start;
    }

    volatile i64 LinearAllocator::g_total_commited_bytes = 0;

    void LinearAllocator::deallocate(void* ptr) { /*everything should be "deallocated" with reset()*/ }
    void* LinearAllocator::reallocate(void* ptr, size_t new_size, size_t old_size, size_t align) {
        if (!ptr) return allocate(new_size, align);
        // realloc not supported
        ASSERT(false);
        return nullptr;
    }
}