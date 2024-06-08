#include "TagAllocator.hpp"

namespace cal {

    TagAllocator::TagAllocator(IAllocator& allocator, const char* tag_name)
        : m_tag(tag_name)
    {
        m_effective_allocator = m_direct_parent = &allocator;
        while (m_effective_allocator->getParent() && m_effective_allocator->isTagAllocator()) {
            m_effective_allocator = m_effective_allocator->getParent();
        }
    }

    thread_local TagAllocator* TagAllocator::active_allocator = nullptr;

    void* TagAllocator::allocate(size_t size, size_t align) {
        active_allocator = this;
        return m_effective_allocator->allocate(size, align);
    }

    void TagAllocator::deallocate(void* ptr) {
        m_effective_allocator->deallocate(ptr);
    }

    void* TagAllocator::reallocate(void* ptr, size_t new_size, size_t old_size, size_t align) {
        active_allocator = this;
        return m_effective_allocator->reallocate(ptr, new_size, old_size, align);
    }
}