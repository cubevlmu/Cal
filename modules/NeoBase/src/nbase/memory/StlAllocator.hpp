// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/memory/Memory.hpp>

namespace neo {

	template<class T>
	class StlAllocator
	{
	public:
		typedef unsigned long long size_type;
		typedef long long difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

	public:
		StlAllocator() = default;
		StlAllocator(const StlAllocator&) = default;

		pointer allocate(size_type n, const void* = 0)
		{
			return (pointer)neo::alloc(n * sizeof(T));
		}

		void deallocate(void* p, size_type)
		{
			neo::free(p);
		}

		pointer address(reference x) const
		{
			return &x;
		}

		const_pointer address(const_reference x) const
		{
			return &x;
		}

		StlAllocator<T>& operator=(const StlAllocator&)
		{
			return *this;
		}

		void construct(pointer p, const T& val)
		{
			new((T*)p) T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}

		size_type max_size() const
		{
			return size_type(-1);
		}

		template<class U>
		struct rebind
		{
			typedef StlAllocator<U> other;
		};

		template<class U>
		StlAllocator(const StlAllocator<U>&)
		{
		}

		template<class U>
		StlAllocator& operator=(const StlAllocator<U>&)
		{
			return *this;
		}
	};
}