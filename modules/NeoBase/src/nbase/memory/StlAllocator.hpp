/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>

#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <sstream>

namespace neo {

	void* alloc(psize size);
	void* allocAligned(psize size, psize align);
	void free(void*);

	
	template <class T>
	class NeoStlAllocator {
	public:
		using value_type = T;

		// Standard typedefs
		using pointer = T*;
		using const_pointer = const T*;
		using void_pointer = void*;
		using const_void_pointer = const void*;

		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		// Allocator traits requirement
		template <class U>
		struct rebind {
			using other = NeoStlAllocator<U>;
		};

		// Constructors
		NeoStlAllocator() noexcept {}
		template <class U>
		NeoStlAllocator(const NeoStlAllocator<U>&) noexcept {}

		// Allocate memory using neo::alloc (must return aligned memory)
		pointer allocate(size_type n) {
			return static_cast<pointer>(neo::alloc(n * sizeof(T)));
		}

		// Deallocate using neo::free
		void deallocate(pointer p, size_type) noexcept {
			neo::free(p);
		}

		// Stateless allocators are equal
		bool operator==(const NeoStlAllocator&) const noexcept { return true; }
		bool operator!=(const NeoStlAllocator&) const noexcept { return false; }
	};


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

	template <typename T>
	using BasicString = std::basic_string<T, std::char_traits<T>, neo::NeoStlAllocator<T>>;
	using String = BasicString<char>;
	using StringView = std::basic_string_view<char>;

	template<typename T>
	using Vector = std::vector<T, neo::NeoStlAllocator<T>>;

	template<typename T>
	using Deque = std::deque<T, neo::NeoStlAllocator<T>>;

	template<typename T>
	using Stack = std::stack<T, neo::Deque<T>>;

	template<typename T>
	using Queue = std::queue<T, neo::Deque<T>>;

	template<typename T, typename Compare = std::less<T>>
	using PriorityQueue = std::priority_queue<T, neo::Vector<T>, Compare>;

	template <typename Key, typename Val>
	using Map = std::map<
		Key,
		Val,
		std::less<Key>,
		neo::NeoStlAllocator<std::pair<const Key, Val>>
	>;

	template <typename Key, typename Val>
	using HashMap = std::unordered_map<
		Key,
		Val,
		std::hash<Key>,
		std::equal_to<Key>,
		neo::NeoStlAllocator<std::pair<const Key, Val>>
	>;

	using OStringStream = std::basic_ostringstream<char, std::char_traits<char>, neo::NeoStlAllocator<char>>;
}