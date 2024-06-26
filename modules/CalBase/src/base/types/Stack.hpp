#pragma once

#include "base/allocator/IAllocator.hpp"
#include "Span.hpp"
#include <functional>

namespace cal {

    template <typename T>
    struct Stack {
        explicit Stack(IAllocator& allocator)
            : m_allocator(allocator)
            , m_data(nullptr)
            , m_capacity(0)
            , m_size() {}

        Stack(Stack&& rhs)
            : m_allocator(rhs.m_allocator)
            , m_data(nullptr)
            , m_capacity(0)
            , m_size(0) {
            swap(rhs);
        }

        Stack(const Stack& rhs) = delete;
        void operator=(const Stack& rhs) = delete;

        Stack<T>&& move() { return static_cast<Stack<T>&&>(*this); }

        T* data() const { return m_data; }
        T* begin() const { return m_data; }
        T* end() const { return m_data ? m_data + m_size : nullptr; }

        const T& top() const { ASSERT(m_size > 0); return m_data[m_size - 1]; }
        T& top() { ASSERT(m_size > 0); return m_data[m_size - 1]; }

        void swap(Stack<T>& rhs) {
            ASSERT(&rhs.m_allocator == &m_allocator);

            u32 i = rhs.m_capacity;
            rhs.m_capacity = m_capacity;
            m_capacity = i;

            i = m_size;
            m_size = rhs.m_size;
            rhs.m_size = i;

            T* p = rhs.m_data;
            rhs.m_data = m_data;
            m_data = p;
        }

        void operator=(Stack&& rhs) {
            ASSERT(&m_allocator == &rhs.m_allocator);
            if (this != &rhs) {
                callDestructors(m_data, m_data + m_size);
                m_allocator.deallocate(m_data);
                m_data = rhs.m_data;
                m_capacity = rhs.m_capacity;
                m_size = rhs.m_size;
                rhs.m_data = nullptr;
                rhs.m_capacity = 0;
                rhs.m_size = 0;
            }
        }

        ~Stack() {
            if (!m_data) return;
            callDestructors(m_data, m_data + m_size);
            m_allocator.deallocate(m_data);
        }

        bool empty() const { return m_size == 0; }

        void clear() {
            callDestructors(m_data, m_data + m_size);
            m_size = 0;
        }

        void pop() {
            if (m_size > 0) {
                m_data[m_size - 1].~T();
                --m_size;
            }
        }

        void push(T&& value) {
            u32 size = m_size;
            if (size == m_capacity) grow();
            new (NewPlaceholder(), (T*)(m_data + size)) T(static_cast<T&&>(value));
            ++size;
            m_size = size;
        }

        void push(const T& value) {
            u32 size = m_size;
            if (size == m_capacity) grow();
            new (NewPlaceholder(), (T*)(m_data + size)) T(value);
            ++size;
            m_size = size;
        }

        u32 byte_size() const { return m_size * sizeof(T); }
        i32 size() const { return m_size; }
        u32 capacity() const { return m_capacity; }

        IAllocator& getAllocator() const { return m_allocator; }

    protected:
        void grow() { reserve(m_capacity < 4 ? 4 : m_capacity + m_capacity / 2); }

        void callDestructors(T* begin, T* end) {
            for (; begin < end; ++begin) {
                begin->~T();
            }
        }

        void reserve(u32 capacity) {
            if (capacity > m_capacity) {
                T* new_data = (T*)m_allocator.allocate(capacity * sizeof(T), alignof(T));
                moveRange(new_data, m_data, m_size);
                m_allocator.deallocate(m_data);
                m_data = new_data;
                m_capacity = capacity;
            }
        }

        static void moveRange(T* dst, T* src, u32 count) {
            ASSERT(dst > src || dst + count < src);
            if constexpr (__is_trivially_copyable(T)) {
                memcpy(dst, src, sizeof(T) * count);
            } else {
                for (u32 i = count - 1; i < count; --i) {
                    new (NewPlaceholder(), dst + i) T(static_cast<T&&>(src[i]));
                    src[i].~T();
                }
            }
        }

        IAllocator& m_allocator;
        u32 m_capacity;
        u32 m_size = 0;
        T* m_data = nullptr;
    };

}
