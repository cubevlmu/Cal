#pragma once

#include "IAllocator.hpp"
#include "base/threading/Atomic.hpp"
#include "base/types/RingBuffer.hpp"

namespace cal {

    struct PageAllocator final
    {
    public:
        enum { PAGE_SIZE = 4096 };

        PageAllocator(IAllocator& fallback);
        ~PageAllocator();

        void* allocate(bool lock);
        void deallocate(void* mem, bool lock);
        u32 getAllocatedCount() const { return allocated_count; }
        u32 getReservedCount() const { return reserved_count; }

        void lock();
        void unlock();

    private:
        volatile i32 allocated_count = 0;
        u32 reserved_count = 0;
        RingBuffer<void*, 512> free_pages;
        Mutex mutex;
    };


    template <typename T>
    struct PagedListIterator
    {
        PagedListIterator(T* value) : value(value) {}

        PagedListIterator(const PagedListIterator& rhs) = delete;
        void operator=(const PagedListIterator& rhs) = delete;

        T* next() {
            for (;;) {
                volatile T* tmp = value;
                if (!tmp) return nullptr;
                if (compareAndExchange64((volatile i64*)&value, (i64)tmp->header.next, (i64)tmp)) return (T*)tmp;
            }
        }

        volatile T* value;
    };


    template <typename T>
    struct PagedList
    {
        PagedList(PageAllocator& allocator)
            : allocator(allocator)
        {}


        ~PagedList()
        {
            T* i = begin;
            while (i) {
                T* tmp = i;
                i = i->header.next;
                allocator.deallocate(tmp, true);
            }
        }


        PagedList(const PagedList& rhs) = delete;
        void operator=(const PagedList& rhs) = delete;


        T* detach()
        {
            T* tmp = begin;
            begin = end = nullptr;
            return tmp;
        }


        T* push()
        {
            allocator.lock();
            void* mem = allocator.allocate(false);
            T* page = new (NewPlaceholder(), mem) T;
            if (!begin) {
                begin = end = page;
            }
            else {
                end->header.next = page;
                page->header.next = nullptr;
                end = page;
            }
            allocator.unlock();
            return page;
        }


        T* begin = nullptr;
        T* end = nullptr;
        PageAllocator& allocator;
    };


}