#include "BaseProxyAllocator.hpp"

#include "base/threading/Atomic.hpp"

namespace cal {

    BaseProxyAllocator::BaseProxyAllocator(IAllocator& source)
        : m_source(source)
    {
        m_allocation_count = 0;
    }

    BaseProxyAllocator::~BaseProxyAllocator() { ASSERT(m_allocation_count == 0); }


    void* BaseProxyAllocator::allocate(size_t size, size_t align)
    {
        atomicIncrement(&m_allocation_count);
        return m_source.allocate(size, align);
    }


    void BaseProxyAllocator::deallocate(void* ptr)
    {
        if (ptr)
        {
            atomicDecrement(&m_allocation_count);
            m_source.deallocate(ptr);
        }
    }


    void* BaseProxyAllocator::reallocate(void* ptr, size_t new_size, size_t old_size, size_t align)
    {
        if (!ptr) atomicIncrement(&m_allocation_count);
        if (new_size == 0) atomicDecrement(&m_allocation_count);
        return m_source.reallocate(ptr, new_size, old_size, align);
    }

}