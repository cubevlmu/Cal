#pragma once

#include "IAllocator.hpp"

namespace cal {

    // detects memory leaks, just by counting number of allocations - very fast
    struct BaseProxyAllocator final : IAllocator {
        explicit BaseProxyAllocator(IAllocator& source);
        ~BaseProxyAllocator();

        void* allocate(size_t size, size_t align) override;
        void deallocate(void* ptr) override;
        void* reallocate(void* ptr, size_t new_size, size_t old_size, size_t align) override;
        IAllocator& getSourceAllocator() { return m_source; }

        IAllocator* getParent() const override { return &m_source; }

    private:
        IAllocator& m_source;
        volatile i32 m_allocation_count;
    };

}