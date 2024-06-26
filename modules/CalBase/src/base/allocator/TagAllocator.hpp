#pragma once

#include "IAllocator.hpp"

namespace cal {

    // set active_allocator before calling its parent allocator, 
    // parent allocator can use the tag to e.g. group allocations
    struct TagAllocator final : IAllocator {
        TagAllocator(IAllocator& allocator, const char* tag_name);

        void* allocate(size_t size, size_t align) override;
        void deallocate(void* ptr) override;
        void* reallocate(void* ptr, size_t new_size, size_t old_size, size_t align) override;

        IAllocator* getParent() const override { return m_direct_parent; }
        bool isTagAllocator() const override { return true; }

        IAllocator* m_direct_parent;
        // skip TagAllocator parents when actually allocating
        IAllocator* m_effective_allocator;
        const char* m_tag;

        static thread_local TagAllocator* active_allocator;
    };
}