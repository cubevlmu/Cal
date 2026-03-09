/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/memory/Allocation.hpp>

namespace neo {

	// Template for dynamic array with variable capacity.
	template<typename T, typename AllocationType = HeapAllocation>
	class NArray
	{
		friend NArray;
	public:
		using ItemType = T;
		using AllocationData = typename AllocationType::template Data<T>;

	private:
		i32 m_count;
		i32 m_capacity;
		AllocationData m_allocation;

		NE_FORCE_INLINE static void moveToEmpty(AllocationData& to, AllocationData& from, const i32 fromCount, const i32 fromCapacity)
		{
			if constexpr (AllocationType::HasSwap)
				to.swapVal(from);
			else
			{
				to.allocate(fromCapacity);
				memory::moveItems(to.get(), from.get(), fromCount);
				memory::destructItems(from.get(), fromCount);
				from.free();
			}
		}

	public:
		NE_FORCE_INLINE NArray()
			: m_count(0)
			, m_capacity(0)
		{
		}


		explicit NArray(const i32 capacity)
			: m_count(0)
			, m_capacity(capacity)
		{
			if (capacity > 0)
				m_allocation.allocate(capacity);
		}


		NArray(std::initializer_list<T> initList)
		{
			m_count = m_capacity = static_cast<i32>(initList.size());
			if (m_count > 0)
			{
				m_allocation.allocate(m_count);
				memory::constructItems(m_allocation.get(), initList.begin(), m_count);
			}
		}


		NArray(const T* data, const i32 length)
		{
			NE_ASSERT(length >= 0);
			m_count = m_capacity = length;
			if (length > 0)
			{
				m_allocation.allocate(length);
				memory::constructItems(m_allocation.get(), data, length);
			}
		}


		NArray(const NArray& other)
		{
			m_count = m_capacity = other.m_count;
			if (m_capacity > 0)
			{
				m_allocation.allocate(m_capacity);
				memory::constructItems(m_allocation.get(), other.get(), other.m_count);
			}
		}


		NArray(const NArray& other, i32 extraSize)
		{
			NE_ASSERT(extraSize >= 0);
			m_count = m_capacity = other.m_count + extraSize;
			if (m_capacity > 0)
			{
				m_allocation.allocate(m_capacity);
				memory::constructItems(m_allocation.get(), other.get(), other.m_count);
				memory::constructItems(m_allocation.get() + other.m_count, extraSize);
			}
		}


		template<typename OtherT = T, typename OtherAllocationType = AllocationType>
		explicit NArray(const NArray<OtherT, OtherAllocationType>& other) noexcept
		{
			m_capacity = other.Capacity();
			m_count = other.count();
			if (m_capacity > 0)
			{
				m_allocation.allocate(m_capacity);
				memory::constructItems(m_allocation.get(), other.get(), m_count);
			}
		}


		NArray(NArray&& other) noexcept
		{
			m_count = other.m_count;
			m_capacity = other.m_capacity;
			other.m_count = 0;
			other.m_capacity = 0;
			moveToEmpty(m_allocation, other.m_allocation, m_count, m_capacity);
		}
		NArray& operator=(std::initializer_list<T> initList) noexcept
		{
			clear();
			if (initList.size() > 0)
			{
				ensureCapacity(static_cast<i32>(initList.size()));
				m_count = static_cast<i32>(initList.size());
				memory::constructItems(m_allocation.get(), initList.begin(), m_count);
			}
			return *this;
		}
		NArray& operator=(const NArray& other) noexcept
		{
			if (this != &other)
			{
				memory::destructItems(m_allocation.get(), m_count);
				if (m_capacity < other.count())
				{
					m_allocation.free();
					m_capacity = other.count();
					m_allocation.allocate(m_capacity);
				}
				m_count = other.count();
				memory::constructItems(m_allocation.get(), other.get(), m_count);
			}
			return *this;
		}
		NArray& operator=(NArray&& other) noexcept
		{
			if (this != &other)
			{
				memory::destructItems(m_allocation.get(), m_count);
				m_allocation.free();
				m_count = other.m_count;
				m_capacity = other.m_capacity;
				other.m_count = 0;
				other.m_capacity = 0;
				moveToEmpty(m_allocation, other.m_allocation, m_count, m_capacity);
			}
			return *this;
		}

		~NArray()
		{
			memory::destructItems(m_allocation.get(), m_count);
		}

	public:
		NE_FORCE_INLINE i32 count() const
		{
			return m_count;
		}
		NE_FORCE_INLINE i32 capacity() const
		{
			return m_capacity;
		}
		NE_FORCE_INLINE bool hasItems() const
		{
			return m_count != 0;
		}
		NE_FORCE_INLINE bool isEmpty() const
		{
			return m_count == 0;
		}

		bool isValidIndex(const i32 index) const
		{
			return index < m_count && index >= 0;
		}

		NE_FORCE_INLINE T* get()
		{
			return m_allocation.get();
		}

		NE_FORCE_INLINE const T* get() const
		{
			return m_allocation.get();
		}

		NE_FORCE_INLINE T& at(const i32 index)
		{
			NE_ASSERT(index >= 0 && index < m_count);
			return m_allocation.get()[index];
		}
		NE_FORCE_INLINE const T& at(const i32 index) const
		{
			NE_ASSERT(index >= 0 && index < m_count);
			return m_allocation.get()[index];
		}
		NE_FORCE_INLINE T& operator[](const i32 index)
		{
			NE_ASSERT(index >= 0 && index < m_count);
			return m_allocation.get()[index];
		}
		NE_FORCE_INLINE const T& operator[](const i32 index) const
		{
			NE_ASSERT(index >= 0 && index < m_count);
			return m_allocation.get()[index];
		}

		NE_FORCE_INLINE T& last()
		{
			NE_ASSERT(m_count > 0);
			return m_allocation.get()[m_count - 1];
		}
		NE_FORCE_INLINE const T& last() const
		{
			NE_ASSERT(m_count > 0);
			return m_allocation.get()[m_count - 1];
		}

		NE_FORCE_INLINE T& first()
		{
			NE_ASSERT(m_count > 0);
			return m_allocation.get()[0];
		}
		NE_FORCE_INLINE const T& first() const
		{
			NE_ASSERT(m_count > 0);
			return m_allocation.get()[0];
		}

		NE_FORCE_INLINE const T& back() const
		{
			return last();
		}
		NE_FORCE_INLINE T& back()
		{
			return last();
		}

	public:
		NE_FORCE_INLINE T* begin()
		{
			return &m_allocation.get()[0];
		}
		NE_FORCE_INLINE T* end()
		{
			return &m_allocation.get()[m_count];
		}
		NE_FORCE_INLINE const T* begin() const
		{
			return &m_allocation.get()[0];
		}
		NE_FORCE_INLINE const T* end() const
		{
			return &m_allocation.get()[m_count];
		}

	public:
		NE_FORCE_INLINE void clear()
		{
			memory::destructItems(m_allocation.get(), m_count);
			m_count = 0;
		}

#if defined(_MSC_VER)
		template<typename U = T, typename = typename NEnableIf<NIsPointer<U>::Value>::Type>
#endif
		void clearDelete()
		{
			T* data = get();
			for (i32 i = 0; i < m_count; i++)
			{
				if (data[i])
					deletePtr(data[i]);
			}
			clear();
		}

		void setCapacity(const i32 capacity, const bool preserveContents = true)
		{
			if (capacity == m_capacity)
				return;
			NE_ASSERT(capacity >= 0);
			const i32 count = preserveContents ? (m_count < capacity ? m_count : capacity) : 0;
			m_allocation.relocate(capacity, m_count, count);
			m_capacity = capacity;
			m_count = count;
		}

		void resize(const i32 size, const bool preserveContents = true)
		{
			if (m_count > size)
				memory::destructItems(m_allocation.get() + size, m_count - size);
			else
			{
				ensureCapacity(size, preserveContents);
				memory::constructItems(m_allocation.get() + m_count, size - m_count);
			}
			m_count = size;
		}

		void ensureCapacity(const i32 minCapacity, const bool preserveContents = true)
		{
			if (m_capacity < minCapacity)
			{
				const i32 capacity = m_allocation.calculateCapacityGrow(m_capacity, minCapacity);
				setCapacity(capacity, preserveContents);
			}
		}
		NE_FORCE_INLINE void reserve(u32 capacity) {
			ensureCapacity(capacity);
		}

		void setAll(const T& value)
		{
			T* data = m_allocation.get();
			for (i32 i = 0; i < m_count; i++)
				data[i] = value;
		}

		void set(const T* data, const i32 count)
		{
			ensureCapacity(count, false);
			memory::destructItems(m_allocation.get(), m_count);
			m_count = count;
			memory::constructItems(m_allocation.get(), data, m_count);
		}

		void add(const T& item)
		{
			ensureCapacity(m_count + 1);
			memory::constructItems(m_allocation.get() + m_count, &item, 1);
			++m_count;
		}
		void add(T&& item)
		{
			ensureCapacity(m_count + 1);
			memory::moveItems(m_allocation.get() + m_count, &item, 1);
			++m_count;
		}
		void add(const T* items, const i32 count)
		{
			ensureCapacity(m_count + count);
			memory::constructItems(m_allocation.get() + m_count, items, count);
			m_count += count;
		}
		template<typename OtherT, typename OtherAllocationType = AllocationType>
		NE_FORCE_INLINE void add(const NArray<OtherT, OtherAllocationType>& other)
		{
			add(other.get(), other.count());
		}

		NE_FORCE_INLINE void addUnique(const T& item)
		{
			if (!contains(item))
				add(item);
		}

		NE_FORCE_INLINE void addDefault(const i32 count = 1)
		{
			ensureCapacity(m_count + count);
			memory::constructItems(m_allocation.get() + m_count, count);
			m_count += count;
		}

		NE_FORCE_INLINE void addUninitialized(const i32 count = 1)
		{
			ensureCapacity(m_count + count);
			m_count += count;
		}

		NE_FORCE_INLINE T& addOne()
		{
			ensureCapacity(m_count + 1);
			memory::constructItems(m_allocation.get() + m_count, 1);
			++m_count;
			return m_allocation.get()[m_count - 1];
		}

		void addZeroed(const i32 count = 1)
		{
			ensureCapacity(m_count + count);
			memset(m_allocation.get() + m_count, 0, count * sizeof(T));
			m_count += count;
		}

		void insert(const i32 index, const T& item)
		{
			NE_ASSERT(index >= 0 && index <= m_count);
			ensureCapacity(m_count + 1);
			T* data = m_allocation.get();
			memory::constructItems(data + m_count, 1);
			for (i32 i = m_count - 1; i >= index; i--)
				data[i + 1] = data[i];
			m_count++;
			data[index] = item;
		}
		void insert(const i32 index, T&& item)
		{
			NE_ASSERT(index >= 0 && index <= m_count);
			ensureCapacity(m_count + 1);
			T* data = m_allocation.get();
			memory::constructItems(data + m_count, 1);
			for (i32 i = m_count - 1; i >= index; i--)
				data[i + 1] = moveTemp(data[i]);
			++m_count;
			data[index] = moveTemp(item);
		}
		void insert(const i32 index)
		{
			NE_ASSERT(index >= 0 && index <= m_count);
			ensureCapacity(m_count + 1);
			T* data = m_allocation.get();
			memory::constructItems(data + m_count, 1);
			for (i32 i = m_count - 1; i >= index; i--)
				data[i + 1] = data[i];
			++m_count;
		}

		template<typename TComparableType>
		bool contains(const TComparableType& item) const
		{
			const T* data = m_allocation.get();
			for (i32 i = 0; i < m_count; i++)
			{
				if (data[i] == item)
					return true;
			}
			return false;
		}

		bool removeKeepOrder(const T& item)
		{
			const i32 index = find(item);
			if (index == -1)
				return true;
			removeAtKeepOrder(index);
			return false;
		}

		void removeAllKeepOrder(const T& item)
		{
			for (i32 i = count() - 1; i >= 0; --i)
			{
				if (m_allocation.get()[i] == item)
				{
					removeAtKeepOrder(i);
					if (isEmpty())
						break;
				}
			}
		}

		void removeAtKeepOrder(const i32 index)
		{
			NE_ASSERT(index < m_count && index >= 0);
			--m_count;
			T* data = m_allocation.get();
			if (index < m_count)
			{
				T* dst = data + index;
				T* src = data + (index + 1);
				const i32 count = m_count - index;
				for (i32 i = 0; i < count; ++i)
					dst[i] = moveTemp(src[i]);
			}
			memory::destructItems(data + m_count, 1);
		}

		bool remove(const T& item)
		{
			const i32 index = find(item);
			if (index == -1)
				return true;
			removeAt(index);
			return false;
		}

		void removeAll(const T& item)
		{
			for (i32 i = count() - 1; i >= 0; --i)
			{
				if (m_allocation.get()[i] == item)
				{
					removeAt(i);
					if (isEmpty())
						break;
				}
			}
		}

		void removeAt(const i32 index)
		{
			NE_ASSERT(index < m_count && index >= 0);
			--m_count;
			T* data = m_allocation.get();
			if (m_count)
				data[index] = data[m_count];
			memory::destructItems(data + m_count, 1);
		}

		void removeLast()
		{
			NE_ASSERT(m_count > 0);
			--m_count;
			memory::destructItems(m_allocation.get() + m_count, 1);
		}

		void swap(NArray& other)
		{
			if constexpr (AllocationType::HasSwap)
			{
				m_allocation.swapVal(other.m_allocation);
				neo::swapVal(m_count, other.m_count);
				neo::swapVal(m_capacity, other.m_capacity);
			}
			else
			{
				neo::swapVal(other, *this);
			}
		}

		void reverse()
		{
			T* data = m_allocation.get();
			const i32 count = m_count / 2;
			for (i32 i = 0; i < count; ++i)
				::neo::swapVal(data[i], data[m_count - i - 1]);
		}

	public:
		NE_FORCE_INLINE void push(const T& item)
		{
			add(item);
		}

		NE_FORCE_INLINE T pop()
		{
			T item = moveTemp(last());
			removeLast();
			return item;
		}

		NE_FORCE_INLINE T& peek()
		{
			NE_ASSERT(m_count > 0);
			return m_allocation.get()[m_count - 1];
		}

		NE_FORCE_INLINE const T& peek() const
		{
			NE_ASSERT(m_count > 0);
			return m_allocation.get()[m_count - 1];
		}

	public:
		void enqueue(const T& item)
		{
			add(item);
		}

		void enqueue(T&& item)
		{
			add(moveTemp(item));
		}

		T dequeue()
		{
			NE_ASSERT(hasItems());
			T item = moveTemp(m_allocation.get()[0]);
			removeAtKeepOrder(0);
			return item;
		}

	public:
		template<typename ComparableType>
		NE_FORCE_INLINE bool find(const ComparableType& item, i32& index) const
		{
			index = find(item);
			return index != -1;
		}

		template<typename ComparableType>
		i32 find(const ComparableType& item) const
		{
			if (m_count > 0)
			{
				const T* NE_RESTRICT start = m_allocation.get();
				for (const T* NE_RESTRICT data = start, *NE_RESTRICT dataEnd = data + m_count; data != dataEnd; ++data)
				{
					if (*data == item)
						return static_cast<i32>(data - start);
				}
			}
			return -1;
		}

		template<typename ComparableType>
		NE_FORCE_INLINE bool findLast(const ComparableType& item, int& index) const
		{
			index = findLast(item);
			return index != -1;
		}

		template<typename ComparableType>
		i32 findLast(const ComparableType& item) const
		{
			if (m_count > 0)
			{
				const T* NE_RESTRICT end = m_allocation.get() + m_count;
				for (const T* NE_RESTRICT data = end, *NE_RESTRICT dataStart = data - m_count; data != dataStart;)
				{
					--data;
					if (*data == item)
						return static_cast<i32>(data - dataStart);
				}
			}
			return -1;
		}

	public:
		template<typename OtherT = T, typename OtherAllocationType = AllocationType>
		bool operator==(const NArray<OtherT, OtherAllocationType>& other) const
		{
			if (m_count == other.count())
			{
				const T* data = m_allocation.get();
				const T* otherData = other.get();
				for (i32 i = 0; i < m_count; i++)
				{
					if (!(data[i] == otherData[i]))
						return false;
				}
				return true;
			}
			return false;
		}

		template<typename OtherT = T, typename OtherAllocationType = AllocationType>
		bool operator!=(const NArray<OtherT, OtherAllocationType>& other) const
		{
			return !operator==(other);
		}

	public:
		// The collection iterator
		struct Iterator
		{
			friend NArray;
		private:
			NArray* m_array;
			i32 m_index;

			Iterator(NArray* array, const i32 index)
				: m_array(array)
				, m_index(index)
			{
			}

			Iterator(const NArray* array, const i32 index)
				: m_array(const_cast<NArray*>(array))
				, m_index(index)
			{
			}

		public:
			Iterator()
				: m_array(nullptr)
				, m_index(-1)
			{
			}

			Iterator(const Iterator& i)
				: m_array(i.m_array)
				, m_index(i.m_index)
			{
			}

			Iterator(Iterator&& i)
				: m_array(i.m_array)
				, m_index(i.m_index)
			{
			}

		public:
			NE_FORCE_INLINE NArray* getNArray() const
			{
				return m_array;
			}

			NE_FORCE_INLINE i32 getIndex() const
			{
				return m_index;
			}

			NE_FORCE_INLINE bool isEnd() const
			{
				return m_index == m_array->m_count;
			}

			NE_FORCE_INLINE bool isNotEnd() const
			{
				return m_index != m_array->m_count;
			}

			NE_FORCE_INLINE T& operator*() const
			{
				return m_array->get()[m_index];
			}

			NE_FORCE_INLINE T* operator->() const
			{
				return &m_array->get()[m_index];
			}

			NE_FORCE_INLINE bool operator==(const Iterator& v) const
			{
				return m_array == v.m_array && m_index == v.m_index;
			}

			NE_FORCE_INLINE bool operator!=(const Iterator& v) const
			{
				return m_array != v.m_array || m_index != v.m_index;
			}

			Iterator& operator=(const Iterator& v)
			{
				m_array = v.m_array;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator=(Iterator&& v)
			{
				m_array = v.m_array;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator++()
			{
				if (m_index != m_array->m_count)
					m_index++;
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator temp = *this;
				if (m_index != m_array->m_count)
					m_index++;
				return temp;
			}

			Iterator& operator--()
			{
				if (m_index > 0)
					--m_index;
				return *this;
			}

			Iterator operator--(int)
			{
				Iterator temp = *this;
				if (m_index > 0)
					--m_index;
				return temp;
			}
		};

	public:
		NE_FORCE_INLINE Iterator Begin() const
		{
			return Iterator(this, 0);
		}
		NE_FORCE_INLINE Iterator End() const
		{
			return Iterator(this, m_count);
		}
	};

}


template<typename T, typename AllocationType>
void* operator new(const size_t size, neo::NArray<T, AllocationType>& array)
{
	using namespace neo;
	NE_ASSERT(size == sizeof(T));
	const i32 index = array.count();
	array.addUninitialized(1);
	return &array[index];
}

template<typename T, typename AllocationType>
void* operator new(const size_t size, neo::NArray<T, AllocationType>& array, const neo::i32 index)
{
	using namespace neo;
	NE_ASSERT(size == sizeof(T));
	array.insert(index);
	return &array[index];
}