/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/memory/Allocation.hpp>
#include <nbase/utils/HashFunc.hpp>

namespace neo {

	template<typename T, typename AllocationType = HeapAllocation>
	class NArray;

	enum class BucketState : byte
	{
		Empty = 0,
		Deleted = 1,
		Occupied = 2,
	};

#ifndef HASHMAP_DEFAULT_CAPACITY
#if PLATFORM_DESKTOP
#define HASHMAP_DEFAULT_CAPACITY 256
#else
#define HASHMAP_DEFAULT_CAPACITY 64
#endif
#endif

// Default slack space divider for the dictionaries.
#define HASHMAP_DEFAULT_SLACK_SCALE 3
// Function for dictionary that tells how change hash index during iteration (size param is a buckets table size).
#define HASHMAP_PROB_FUNC(size, numChecks) (numChecks)
//#define HASHMAP_PROB_FUNC(size, numChecks) (1)

	// Template for unordered dictionary with mapped key with value pairs
	template<typename KeyType, typename ValueType, typename AllocationType = HeapAllocation>
	class NHashMap
	{
		friend NHashMap;
	public:
		// Describes single portion of space for the key and value pair in a hash map.
		struct Bucket
		{
			friend NHashMap;
			KeyType Key;
			ValueType Value;

		private:
			BucketState m_state;

			NE_FORCE_INLINE void free()
			{
				if (m_state == BucketState::Occupied)
				{
					memory::destructItem(&Key);
					memory::destructItem(&Value);
				}
				m_state = BucketState::Empty;
			}

			NE_FORCE_INLINE void destroy()
			{
				m_state = BucketState::Deleted;
				memory::destructItem(&Key);
				memory::destructItem(&Value);
			}

			template<typename KeyComparableType>
			NE_FORCE_INLINE void occupy(const KeyComparableType& key)
			{
				memory::constructItems(&Key, &key, 1);
				memory::constructItem(&Value);
				m_state = BucketState::Occupied;
			}

			template<typename KeyComparableType>
			NE_FORCE_INLINE void occupy(const KeyComparableType& key, const ValueType& value)
			{
				memory::constructItems(&Key, &key, 1);
				memory::constructItems(&Value, &value, 1);
				m_state = BucketState::Occupied;
			}

			template<typename KeyComparableType>
			NE_FORCE_INLINE void occupy(const KeyComparableType& key, ValueType&& value)
			{
				memory::constructItems(&Key, &key, 1);
				memory::moveItems(&Value, &value, 1);
				m_state = BucketState::Occupied;
			}

			NE_FORCE_INLINE bool isEmpty() const
			{
				return m_state == BucketState::Empty;
			}

			NE_FORCE_INLINE bool isDeleted() const
			{
				return m_state == BucketState::Deleted;
			}

			NE_FORCE_INLINE bool isOccupied() const
			{
				return m_state == BucketState::Occupied;
			}

			NE_FORCE_INLINE bool isNotOccupied() const
			{
				return m_state != BucketState::Occupied;
			}
		};

		using AllocationData = typename AllocationType::template Data<Bucket>;

	private:
		i32 m_elementsCount = 0;
		i32 m_deletedCount = 0;
		i32 m_size = 0;
		AllocationData m_allocation;

		NE_FORCE_INLINE static void moveToEmpty(AllocationData& to, AllocationData& from, const i32 fromSize)
		{
			if constexpr(AllocationType::HasSwap)
				to.swap(from);
			else
			{
				to.allocate(fromSize);
				Bucket* toData = to.get();
				Bucket* fromData = from.get();
				for (i32 i = 0; i < fromSize; i++)
				{
					Bucket& fromBucket = fromData[i];
					if (fromBucket.isOccupied())
					{
						Bucket& toBucket = toData[i];
						memory::moveItems(&toBucket.Key, &fromBucket.Key, 1);
						memory::moveItems(&toBucket.Value, &fromBucket.Value, 1);
						toBucket.m_state = BucketState::Occupied;
						memory::destructItem(&fromBucket.Key);
						memory::destructItem(&fromBucket.Value);
						fromBucket.m_state = BucketState::Empty;
					}
				}
				from.free();
			}
		}

	public:
		NHashMap() = default;
		explicit NHashMap(const i32 capacity)
		{
			setCapacity(capacity);
		}
		NHashMap(NHashMap&& other) noexcept
		{
			m_elementsCount = other.m_elementsCount;
			m_deletedCount = other.m_deletedCount;
			m_size = other.m_size;
			other.m_elementsCount = 0;
			other.m_deletedCount = 0;
			other.m_size = 0;
			moveToEmpty(m_allocation, other.m_allocation, m_size);
		}
		NHashMap(const NHashMap& other)
		{
			clone(other);
		}

		NHashMap& operator=(const NHashMap& other)
		{
			if (this != &other)
				clone(other);
			return *this;
		}
		NHashMap& operator=(NHashMap&& other) noexcept
		{
			if (this != &other)
			{
				clear();
				m_allocation.free();
				m_elementsCount = other.m_elementsCount;
				m_deletedCount = other.m_deletedCount;
				m_size = other.m_size;
				other.m_elementsCount = 0;
				other.m_deletedCount = 0;
				other.m_size = 0;
				moveToEmpty(m_allocation, other.m_allocation, m_size);
			}
			return *this;
		}

		~NHashMap()
		{
			clear();
		}

	public:
		NE_FORCE_INLINE i32 count() const
		{
			return m_elementsCount;
		}

		NE_FORCE_INLINE i32 capacity() const
		{
			return m_size;
		}

		NE_FORCE_INLINE bool isEmpty() const
		{
			return m_elementsCount == 0;
		}

		NE_FORCE_INLINE bool hasItems() const
		{
			return m_elementsCount != 0;
		}

	public:
		struct Iterator
		{
			friend NHashMap;
		private:
			NHashMap* m_collection;
			i32 m_index;

		public:
			Iterator(NHashMap* collection, const i32 index)
				: m_collection(collection)
				, m_index(index)
			{
			}

			Iterator(NHashMap const* collection, const i32 index)
				: m_collection(const_cast<NHashMap*>(collection))
				, m_index(index)
			{
			}

			Iterator()
				: m_collection(nullptr)
				, m_index(-1)
			{
			}

			Iterator(const Iterator& i)
				: m_collection(i.m_collection)
				, m_index(i.m_index)
			{
			}

			Iterator(Iterator&& i) noexcept
				: m_collection(i.m_collection)
				, m_index(i.m_index)
			{
			}

		public:
			NE_FORCE_INLINE i32 index() const
			{
				return m_index;
			}

			NE_FORCE_INLINE bool isEnd() const
			{
				return m_index == m_collection->m_size;
			}

			NE_FORCE_INLINE bool isNotEnd() const
			{
				return m_index != m_collection->m_size;
			}

			NE_FORCE_INLINE Bucket& operator*() const
			{
				return m_collection->m_allocation.get()[m_index];
			}

			NE_FORCE_INLINE Bucket* operator->() const
			{
				return &m_collection->m_allocation.get()[m_index];
			}

			NE_FORCE_INLINE explicit operator bool() const
			{
				return m_index >= 0 && m_index < m_collection->m_size;
			}

			NE_FORCE_INLINE bool operator!() const
			{
				return !(bool)*this;
			}

			NE_FORCE_INLINE bool operator==(const Iterator& v) const
			{
				return m_index == v.m_index && m_collection == v.m_collection;
			}

			NE_FORCE_INLINE bool operator!=(const Iterator& v) const
			{
				return m_index != v.m_index || m_collection != v.m_collection;
			}

			Iterator& operator=(const Iterator& v)
			{
				m_collection = v.m_collection;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator=(Iterator&& v) noexcept
			{
				m_collection = v.m_collection;
				m_index = v.m_index;
				return *this;
			}

			Iterator& operator++()
			{
				const i32 capacity = m_collection->m_size;
				if (m_index != capacity)
				{
					const Bucket* data = m_collection->m_allocation.get();
					do
					{
						++m_index;
					} while (m_index != capacity && data[m_index].isNotOccupied());
				}
				return *this;
			}

			Iterator operator++(int) const
			{
				Iterator i = *this;
				++i;
				return i;
			}

			Iterator& operator--()
			{
				if (m_index > 0)
				{
					const Bucket* data = m_collection->m_allocation.get();
					do
					{
						--m_index;
					} while (m_index > 0 && data[m_index].isNotOccupied());
				}
				return *this;
			}

			Iterator operator--(int) const
			{
				Iterator i = *this;
				--i;
				return i;
			}
		};

	public:
		// Gets element by the key (will add default ValueType element if key not found).
		template<typename KeyComparableType>
		ValueType& at(const KeyComparableType& key)
		{
			// Check if need to rehash elements (prevent many deleted elements that use too much of capacity)
			if (m_deletedCount > m_size / HASHMAP_DEFAULT_SLACK_SCALE)
				compact();

			// Ensure to have enough memory for the next item (in case of new element insertion)
			ensureCapacity((m_elementsCount + 1) * HASHMAP_DEFAULT_SLACK_SCALE + m_deletedCount);

			// Find location of the item or place to insert it
			FindPositionResult pos;
			findPosition(key, pos);

			// Check if that key has been already added
			if (pos.ObjectIndex != -1)
				return m_allocation.get()[pos.ObjectIndex].Value;

			// Insert
			NE_ASSERT(pos.FreeSlotIndex != -1);
			++m_elementsCount;
			Bucket& bucket = m_allocation.get()[pos.FreeSlotIndex];
			bucket.occupy(key);
			return bucket.Value;
		}

		// Gets the element by the key.
		template<typename KeyComparableType>
		const ValueType& at(const KeyComparableType& key) const
		{
			FindPositionResult pos;
			findPosition(key, pos);
			NE_ASSERT(pos.ObjectIndex != -1);
			return m_allocation.get()[pos.ObjectIndex].Value;
		}

		// Gets or sets the element by the key.
		template<typename KeyComparableType>
		NE_FORCE_INLINE ValueType& operator[](const KeyComparableType& key)
		{
			return at(key);
		}

		// Gets or sets the element by the key.
		template<typename KeyComparableType>
		NE_FORCE_INLINE const ValueType& operator[](const KeyComparableType& key) const
		{
			return at(key);
		}

		// Tries to get element with given key.
		template<typename KeyComparableType>
		bool tryGet(const KeyComparableType& key, ValueType& result) const
		{
			if (isEmpty())
				return false;
			FindPositionResult pos;
			findPosition(key, pos);
			if (pos.ObjectIndex == -1)
				return false;
			result = m_allocation.get()[pos.ObjectIndex].Value;
			return true;
		}

		// Tries to get pointer to the element with given key.
		template<typename KeyComparableType>
		ValueType* tryGet(const KeyComparableType& key) const
		{
			if (isEmpty())
				return nullptr;
			FindPositionResult pos;
			findPosition(key, pos);
			if (pos.ObjectIndex == -1)
				return nullptr;
			return const_cast<ValueType*>(&m_allocation.get()[pos.ObjectIndex].Value); //TODO This one is problematic. I think this entire method should be removed.
		}

	public:
		// Clears the collection but without changing its capacity (all inserted elements: keys and values will be removed).
		void clear()
		{
			if (m_elementsCount + m_deletedCount != 0)
			{
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; i++)
					data[i].free();
				m_elementsCount = m_deletedCount = 0;
			}
		}

#if defined(_MSC_VER)
		template<typename = typename NEnableIf<NIsPointer<ValueType>::Value>::Type>
#endif
		void clearDelete()
		{
			for (Iterator i = Begin(); i.isNotEnd(); ++i)
			{
				if (i->Value)
					deletePtr(i->Value);
			}
			clear();
		}

		// Changes the capacity of the collection.
		void setCapacity(i32 capacity, const bool preserveContents = true)
		{
			if (capacity == this->capacity())
				return;
			NE_ASSERT(capacity >= 0);
			AllocationData oldAllocation;
			moveToEmpty(oldAllocation, m_allocation, m_size);
			const i32 oldSize = m_size;
			const i32 oldElementsCount = m_elementsCount;
			m_deletedCount = m_elementsCount = 0;
			if (capacity != 0 && (capacity & (capacity - 1)) != 0)
			{
				// Align capacity value to the next power of two (http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2)
				capacity--;
				capacity |= capacity >> 1;
				capacity |= capacity >> 2;
				capacity |= capacity >> 4;
				capacity |= capacity >> 8;
				capacity |= capacity >> 16;
				capacity++;
			}
			if (capacity)
			{
				m_allocation.allocate(capacity);
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < capacity; i++)
					data[i].m_state = BucketState::Empty;
			}
			m_size = capacity;
			Bucket* oldData = oldAllocation.get();
			if (oldElementsCount != 0 && capacity != 0 && preserveContents)
			{
				FindPositionResult pos;
				for (i32 i = 0; i < oldSize; i++)
				{
					Bucket& oldBucket = oldData[i];
					if (oldBucket.isOccupied())
					{
						findPosition(oldBucket.Key, pos);
						NE_ASSERT(pos.FreeSlotIndex != -1);
						Bucket* bucket = &m_allocation.get()[pos.FreeSlotIndex];
						memory::moveItems(&bucket->Key, &oldBucket.Key, 1);
						memory::moveItems(&bucket->Value, &oldBucket.Value, 1);
						bucket->m_state = BucketState::Occupied;
						++m_elementsCount;
					}
				}
			}
			if (oldElementsCount != 0)
			{
				for (i32 i = 0; i < oldSize; i++)
					oldData[i].free();
			}
		}

		// Ensures that collection has given capacity.
		void ensureCapacity(i32 minCapacity, const bool preserveContents = true)
		{
			if (m_size >= minCapacity)
				return;
			i32 capacity = m_allocation.calculateCapacityGrow(m_size, minCapacity);
			if (capacity < HASHMAP_DEFAULT_CAPACITY)
				capacity = HASHMAP_DEFAULT_CAPACITY;
			setCapacity(capacity, preserveContents);
		}

		// Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
		void swap(NHashMap& other)
		{
			if constexpr(AllocationType::HasSwap)
			{
				neo::swap(m_elementsCount, other.m_elementsCount);
				neo::swap(m_deletedCount, other.m_deletedCount);
				neo::swap(m_size, other.m_size);
				m_allocation.swap(other.m_allocation);
			}
			else
			{
				neo::swap(other, *this);
			}
		}

	public:
		// Add pair element to the collection.
		template<typename KeyComparableType>
		NE_FORCE_INLINE Bucket* add(const KeyComparableType& key, const ValueType& value)
		{
			Bucket* bucket = onAdd(key);
			bucket->occupy(key, value);
			return bucket;
		}

		// Add pair element to the collection.
		template<typename KeyComparableType>
		NE_FORCE_INLINE Bucket* add(const KeyComparableType& key, ValueType&& value)
		{
			Bucket* bucket = onAdd(key);
			bucket->occupy(key, moveTemp(value));
			return bucket;
		}

		// Add pair element to the collection.
		void add(const Iterator& i)
		{
			NE_ASSERT(i.m_collection != this && i);
			const Bucket& bucket = *i;
			add(bucket.Key, bucket.Value);
		}

		// Removes element with a specified key.
		template<typename KeyComparableType>
		bool remove(const KeyComparableType& key)
		{
			if (isEmpty())
				return false;
			FindPositionResult pos;
			findPosition(key, pos);
			if (pos.ObjectIndex != -1)
			{
				m_allocation.get()[pos.ObjectIndex].destroy();
				--m_elementsCount;
				++m_deletedCount;
				return true;
			}
			return false;
		}

		// Removes element at specified iterator.
		bool remove(const Iterator& i)
		{
			NE_ASSERT(i.m_collection == this);
			if (i)
			{
				NE_ASSERT(m_allocation.get()[i.m_index].isOccupied());
				m_allocation.get()[i.m_index].destroy();
				--m_elementsCount;
				++m_deletedCount;
				return true;
			}
			return false;
		}

		// Removes elements with a specified value
		i32 removeValue(const ValueType& value)
		{
			i32 result = 0;
			for (Iterator i = Begin(); i.isNotEnd(); ++i)
			{
				if (i->Value == value)
				{
					remove(i);
					++result;
				}
			}
			return result;
		}

	public:
		// Finds the element with given key in the collection.
		template<typename KeyComparableType>
		Iterator find(const KeyComparableType& key) const
		{
			if (isEmpty())
				return End();
			FindPositionResult pos;
			findPosition(key, pos);
			return pos.ObjectIndex != -1 ? Iterator(this, pos.ObjectIndex) : End();
		}

		// Checks if given key is in a collection.
		template<typename KeyComparableType>
		bool containsKey(const KeyComparableType& key) const
		{
			if (isEmpty())
				return false;
			FindPositionResult pos;
			findPosition(key, pos);
			return pos.ObjectIndex != -1;
		}

		// Checks if given value is in a collection.
		bool containsValue(const ValueType& value) const
		{
			if (hasItems())
			{
				const Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; ++i)
				{
					if (data[i].isOccupied() && data[i].Value == value)
						return true;
				}
			}
			return false;
		}

		// Searches for the specified object and returns the zero-based index of the first occurrence within the entire dictionary.
		bool keyOf(const ValueType& value, KeyType* key) const
		{
			if (hasItems())
			{
				const Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; ++i)
				{
					if (data[i].isOccupied() && data[i].Value == value)
					{
						if (key)
							*key = data[i].Key;
						return true;
					}
				}
			}
			return false;
		}

	public:
		// Clones other collection into this.
		void clone(const NHashMap& other)
		{
			// TODO: if both key and value are POD types then use raw memory copy for buckets
			clear();
			ensureCapacity(other.capacity(), false);
			for (Iterator i = other.Begin(); i != other.End(); ++i)
				add(i);
		}

		// Gets the keys collection to the output array (will contain unique items).
		template<typename ArrayAllocation>
		void getKeys(NArray<KeyType, ArrayAllocation>& result) const
		{
			for (Iterator i = Begin(); i.isNotEnd(); ++i)
				result.add(i->Key);
		}

		// Gets the values collection to the output array (may contain duplicates).
		template<typename ArrayAllocation>
		void getValues(NArray<ValueType, ArrayAllocation>& result) const
		{
			for (Iterator i = Begin(); i.isNotEnd(); ++i)
				result.add(i->Value);
		}

	public:
		Iterator Begin() const
		{
			Iterator i(this, -1);
			++i;
			return i;
		}

		Iterator End() const
		{
			return Iterator(this, m_size);
		}

		Iterator begin()
		{
			Iterator i(this, -1);
			++i;
			return i;
		}

		NE_FORCE_INLINE Iterator end()
		{
			return Iterator(this, m_size);
		}

		Iterator begin() const
		{
			Iterator i(this, -1);
			++i;
			return i;
		}

		NE_FORCE_INLINE Iterator end() const
		{
			return Iterator(this, m_size);
		}

	private:
		// The result container of the dictionary item lookup searching.
		struct FindPositionResult
		{
			i32 ObjectIndex;
			i32 FreeSlotIndex;
		};

		// Returns a pair of positions: 1st where the object is, 2nd where
		// it would go if you wanted to insert it. 1st is -1
		// if object is not found; 2nd is -1 if it is.
		template<typename KeyComparableType>
		void findPosition(const KeyComparableType& key, FindPositionResult& result) const
		{
			NE_ASSERT(m_size);
			const i32 tableSizeMinusOne = m_size - 1;
			i32 bucketIndex = getHash(key) & tableSizeMinusOne;
			i32 insertPos = -1;
			i32 checksCount = 0;
			const Bucket* data = m_allocation.get();
			result.FreeSlotIndex = -1;
			while (checksCount < m_size)
			{
				// Empty bucket
				const Bucket& bucket = data[bucketIndex];
				if (bucket.isEmpty())
				{
					// Found place to insert
					result.ObjectIndex = -1;
					result.FreeSlotIndex = insertPos == -1 ? bucketIndex : insertPos;
					return;
				}
				// Deleted bucket
				if (bucket.isDeleted())
				{
					// Keep searching but mark to insert
					if (insertPos == -1)
						insertPos = bucketIndex;
				}
					// Occupied bucket by target key
				else if (bucket.Key == key)
				{
					// Found key
					result.ObjectIndex = bucketIndex;
					return;
				}
				++checksCount;
				bucketIndex = (bucketIndex + HASHMAP_PROB_FUNC(m_size, checksCount)) & tableSizeMinusOne;
			}
			result.ObjectIndex = -1;
			result.FreeSlotIndex = insertPos;
		}

		template<typename KeyComparableType>
		Bucket* onAdd(const KeyComparableType& key)
		{
			// Check if need to rehash elements (prevent many deleted elements that use too much of capacity)
			if (m_deletedCount > m_size / HASHMAP_DEFAULT_SLACK_SCALE)
				compact();

			// Ensure to have enough memory for the next item (in case of new element insertion)
			ensureCapacity((m_elementsCount + 1) * HASHMAP_DEFAULT_SLACK_SCALE + m_deletedCount);

			// Find location of the item or place to insert it
			FindPositionResult pos;
			findPosition(key, pos);

			// Ensure key is unknown
			NE_ASSERT(pos.ObjectIndex == -1 && "That key has been already added to the dictionary.");

			// Insert
			NE_ASSERT(pos.FreeSlotIndex != -1);
			m_elementsCount++;
			return &m_allocation.get()[pos.FreeSlotIndex];
		}

		void compact()
		{
			if (m_elementsCount == 0)
			{
				// Fast path if it's empty
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; i++)
					data[i].m_state = BucketState::Empty;
			}
			else
			{
				// Rebuild entire table completely
				AllocationData oldAllocation;
				moveToEmpty(oldAllocation, m_allocation, m_size);
				m_allocation.allocate(m_size);
				Bucket* data = m_allocation.get();
				for (i32 i = 0; i < m_size; i++)
					data[i].m_state = BucketState::Empty;
				Bucket* oldData = oldAllocation.get();
				FindPositionResult pos;
				for (i32 i = 0; i < m_size; i++)
				{
					Bucket& oldBucket = oldData[i];
					if (oldBucket.isOccupied())
					{
						findPosition(oldBucket.Key, pos);
						NE_ASSERT(pos.FreeSlotIndex != -1);
						Bucket* bucket = &m_allocation.get()[pos.FreeSlotIndex];
						memory::moveItems(&bucket->Key, &oldBucket.Key, 1);
						memory::moveItems(&bucket->Value, &oldBucket.Value, 1);
						bucket->m_state = BucketState::Occupied;
					}
				}
				for (i32 i = 0; i < m_size; i++)
					oldData[i].free();
			}
			m_deletedCount = 0;
		}
	};
}