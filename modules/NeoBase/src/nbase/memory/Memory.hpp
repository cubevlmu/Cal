// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <nbase/base/TypeTraits.hpp>

#include <cstring>
#include <new>

namespace neo::memory
{
    // Rounds up the input value to the next power of 2 to be used as bigger memory allocation block. Handles overflow.
    inline i32 roundUpToPowerOf2(i32 capacity)
    {
        capacity--;
        capacity |= capacity >> 1;
        capacity |= capacity >> 2;
        capacity |= capacity >> 4;
        capacity |= capacity >> 8;
        capacity |= capacity >> 16;
        u64 capacity64 = (u64)(capacity + 1) * 2;
        if (capacity64 > kMaxI32)
            capacity64 = kMaxI32;
        return (i32)capacity64;
    }

    // Aligns the input value to the next power of 2 to be used as bigger memory allocation block.
    inline i32 alignToPowerOf2(i32 capacity)
    {
        capacity--;
        capacity |= capacity >> 1;
        capacity |= capacity >> 2;
        capacity |= capacity >> 4;
        capacity |= capacity >> 8;
        capacity |= capacity >> 16;
        capacity++;
        return capacity;
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<!NIsTriviallyConstructible<T>::Value>::Type constructItem(T* dst)
    {
        new(dst) T();
    }

    // Constructs the item in the memory.
    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<NIsTriviallyConstructible<T>::Value>::Type constructItem(T*)
    {
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<!NIsTriviallyConstructible<T>::Value>::Type constructItems(T* dst, i32 count)
    {
        while (count--)
        {
            new(dst) T();
            ++(T*&)dst;
        }
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<NIsTriviallyConstructible<T>::Value>::Type constructItems(T*, i32)
    {
    }

    template<typename T, typename U>
    NE_FORCE_INLINE typename NEnableIf<!NIsBitwiseConstructible<T, U>::Value>::Type constructItems(T* dst, const U* src, i32 count)
    {
        while (count--)
        {
            new(dst) T(*src);
            ++(T*&)dst;
            ++src;
        }
    }

    template<typename T, typename U>
    NE_FORCE_INLINE typename NEnableIf<NIsBitwiseConstructible<T, U>::Value>::Type constructItems(T* dst, const U* src, i32 count)
    {
        memcpy(dst, src, count * sizeof(U));
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<!NIsTriviallyDestructible<T>::Value>::Type destructItem(T* dst)
    {
        dst->~T();
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<NIsTriviallyDestructible<T>::Value>::Type destructItem(T*)
    {}

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<!NIsTriviallyDestructible<T>::Value>::Type destructItems(T* dst, i32 count)
    {
        while (count--)
        {
            dst->~T();
            ++dst;
        }
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<NIsTriviallyDestructible<T>::Value>::Type destructItems(T*, i32)
    {
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<!NIsTriviallyCopyAssignable<T>::Value>::Type copyItems(T* dst, const T* src, i32 count)
    {
        while (count--)
        {
            *dst = *src;
            ++dst;
            ++src;
        }
    }

    template<typename T>
    NE_FORCE_INLINE typename NEnableIf<NIsTriviallyCopyAssignable<T>::Value>::Type copyItems(T* dst, const T* src, i32 count)
    {
        memcpy(dst, src, count * sizeof(T));
    }

    template<typename T, typename U>
    NE_FORCE_INLINE typename NEnableIf<!NIsBitwiseConstructible<T, U>::Value>::Type moveItems(T* dst, const U* src, i32 count)
    {
        while (count--)
        {
            new(dst) T((T&&)*src);
            ++(T*&)dst;
            ++src;
        }
    }

    template<typename T, typename U>
    NE_FORCE_INLINE typename NEnableIf<NIsBitwiseConstructible<T, U>::Value>::Type moveItems(T* dst, const U* src, i32 count)
    {
        memcpy(dst, src, count * sizeof(U));
    }
}


namespace neo {

	void* alloc(psize size);
	void* allocAligned(psize size, psize align);
	void free(void*);

    template<class T>
    inline T *newObject() {
        T *ptr = (T *) neo::allocAligned(sizeof(T), alignof(T));
        memory::constructItem(ptr);
        return ptr;
    }

    template<class T, class... Args>
    inline T *newObject(Args &&...args) {
        T *ptr = (T *) neo::allocAligned(sizeof(T), alignof(T));
        new(ptr) T(forward<Args>(args)...);
        return ptr;
    }

    template<class T>
    inline T *newArray(u32 count) {
        T *ptr = (T *) neo::allocAligned(sizeof(T) * count, alignof(T));
        memory::constructItems(ptr, count);
        return ptr;
    }

    template<class T>
    inline void deletePtr(T *ptr) {
        memory::destructItem(ptr);
        defaultFree(ptr);
    }

    template<class T>
    inline void deleteArray(T *ptr, u32 count) {
        memory::destructItems(ptr, count);
        defaultFree(ptr);
    }

}