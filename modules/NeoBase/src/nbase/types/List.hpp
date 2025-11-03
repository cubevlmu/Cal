// Created by cubevlmu on 2025/10/7.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>

namespace neo {

	template<typename T, typename Pool>
	class NList;

	namespace detail {

		// NList node.
		template<typename T>
		class NListNode
		{
		public:
			T m_value;
			NListNode* m_prev = nullptr;
			NListNode* m_next = nullptr;

			template<typename... TArgs>
			NListNode(TArgs&&... args)
				: m_value(std::forward<TArgs>(args)...)
			{
			}
		};

		// Gets the value of a list node.
		template<typename TNode, typename TValue>
		class GetNListNodeValueFunc
		{
		public:
			TValue& operator()(TNode& node);
			const TValue& operator()(const TNode& node) const;
		};

		// Specialization for NListNode
		template<typename TValue>
		class GetNListNodeValueFunc<NListNode<TValue>, TValue>
		{
		public:
			TValue& operator()(NListNode<TValue>& node)
			{
				return node.m_value;
			}

			const TValue& operator()(const NListNode<TValue>& node) const
			{
				return node.m_value;
			}
		};

		// NList bidirectional iterator.
		template<typename TNodePointer, typename TValuePointer, typename TValueReference, typename TListPointer>
		class NListIterator
		{
			template<typename, typename>
			friend class NListBase;

			template<typename, typename>
			friend class neo::NList;

			template<typename, typename, typename, typename>
			friend class NListIterator;

		public:
			NListIterator() = default;

			NListIterator(const NListIterator& b)
				: m_node(b.m_node)
				, m_list(b.m_list)
			{
			}

			// Allow conversion from iterator to const iterator.
			template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YNList>
			NListIterator(const NListIterator<YNodePointer, YValuePointer, YValueReference, YNList>& b)
				: m_node(b.m_node)
				, m_list(b.m_list)
			{
			}

			NListIterator(TNodePointer node, TListPointer list)
				: m_node(node)
				, m_list(list)
			{
				NE_ASSERT(list);
			}

			NListIterator& operator=(const NListIterator& b)
			{
				m_node = b.m_node;
				m_list = b.m_list;
				return *this;
			}

			TValueReference operator*() const
			{
				NE_ASSERT(m_node);
				using NodeType = typename std::remove_pointer<TNodePointer>::Type;
				using ValueType = typename std::remove_pointer<TValuePointer>::Type;
				return detail::GetNListNodeValueFunc<NodeType, ValueType>()(*m_node);
			}

			TValuePointer operator->() const
			{
				NE_ASSERT(m_node);
				using NodeType = typename std::remove_pointer<TNodePointer>::Type;
				using ValueType = typename std::remove_pointer<TValuePointer>::Type;
				return &detail::GetNListNodeValueFunc<NodeType, ValueType>()(*m_node);
			}

			NListIterator& operator++()
			{
				NE_ASSERT(m_node);
				m_node = m_node->m_next;
				return *this;
			}

			NListIterator operator++(int)
			{
				NE_ASSERT(m_node);
				NListIterator out = *this;
				++(*this);
				return out;
			}

			NListIterator& operator--();

			NListIterator operator--(int)
			{
				NE_ASSERT(m_node);
				NListIterator out = *this;
				--(*this);
				return out;
			}

			NListIterator operator+(u32 n) const
			{
				NListIterator it = *this;
				while (n-- != 0)
				{
					++it;
				}
				return it;
			}

			NListIterator operator-(u32 n) const
			{
				NListIterator it = *this;
				while (n-- != 0)
				{
					--it;
				}
				return it;
			}

			NListIterator& operator+=(u32 n)
			{
				while (n-- != 0)
				{
					++(*this);
				}
				return *this;
			}

			NListIterator& operator-=(u32 n)
			{
				while (n-- != 0)
				{
					--(*this);
				}
				return *this;
			}

			template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YNList>
			bool operator==(const NListIterator<YNodePointer, YValuePointer, YValueReference, YNList>& b) const
			{
				NE_ASSERT(m_list == b.m_list && "Comparing iterators from different lists");
				return m_node == b.m_node;
			}

			template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YNList>
			bool operator!=(const NListIterator<YNodePointer, YValuePointer, YValueReference, YNList>& b) const
			{
				return !(*this == b);
			}

		private:
			TNodePointer m_node = nullptr;
			TListPointer m_list = nullptr; //< Used to go back from the end
		};

		// Double linked list base.
		template<typename T, typename TNode>
		class NListBase
		{
			template<typename, typename, typename, typename>
			friend class NListIterator;

		public:
			using Value = T;
			using Reference = Value&;
			using ConstReference = const Value&;
			using Pointer = Value*;
			using ConstPointer = const Value*;
			using Iterator = NListIterator<TNode*, Pointer, Reference, NListBase*>;
			using ConstIterator = NListIterator<const TNode*, ConstPointer, ConstReference, const NListBase*>;

			NListBase() = default;

			NListBase(NListBase&& b)
			{
				move(b);
			}

			NListBase(const NListBase&) = delete; // Non-copyable

			NListBase& operator=(const NListBase&) = delete; // Non-copyable

			NListBase& operator=(NListBase&& b)
			{
				move(b);
				return *this;
			}

			// Compare with another list.
			bool operator==(const NListBase& b) const;

			// Get first element.
			ConstReference getFront() const
			{
				NE_ASSERT(!isEmpty());
				return detail::GetNListNodeValueFunc<TNode, T>()(*m_head);
			}

			// Get first element.
			Reference getFront()
			{
				NE_ASSERT(!isEmpty());
				return detail::GetNListNodeValueFunc<TNode, T>()(*m_head);
			}

			// Get last element.
			ConstReference getBack() const
			{
				NE_ASSERT(!isEmpty());
				return detail::GetNListNodeValueFunc<TNode, T>()(*m_tail);
			}

			// Get last element.
			Reference getBack()
			{
				NE_ASSERT(!isEmpty());
				return detail::GetNListNodeValueFunc<TNode, T>()(*m_tail);
			}

			// Get begin.
			Iterator getBegin()
			{
				return Iterator(m_head, this);
			}

			// Get begin.
			ConstIterator getBegin() const
			{
				return ConstIterator(m_head, this);
			}

			// Get end.
			Iterator getEnd()
			{
				return Iterator(nullptr, this);
			}

			// Get end.
			ConstIterator getEnd() const
			{
				return ConstIterator(nullptr, this);
			}

			// Get begin.
			Iterator begin()
			{
				return getBegin();
			}

			// Get begin.
			ConstIterator begin() const
			{
				return getBegin();
			}

			// Get end.
			Iterator end()
			{
				return getEnd();
			}

			// Get end.
			ConstIterator end() const
			{
				return getEnd();
			}

			// Return true if list is empty.
			bool isEmpty() const
			{
				return m_head == nullptr;
			}

			// Iterate the list using lambda.
			template<typename TFunc>
			bool iterateForward(TFunc func);

			// Iterate the list backwards using lambda.
			template<typename TFunc>
			bool iterateBackward(TFunc func);

			// Find item.
			Iterator find(const Value& a);

			// Sort the list.
			template<typename TCompFunc = std::less<Value>>
			void sort(TCompFunc compFunc = TCompFunc());

			// Compute the size of elements in the list.
			psize getSize() const;

		protected:
			TNode* m_head = nullptr;
			TNode* m_tail = nullptr;

			void pushBackNode(TNode* node);
			void pushFrontNode(TNode* node);
			void insertNode(TNode* pos, TNode* node);
			void removeNode(TNode* node);
			void popBack();
			void popFront();

		private:
			// Used in sort.
			TNode* swap(TNode* one, TNode* two);

			void move(NListBase& b)
			{
				m_head = b.m_head;
				b.m_head = nullptr;
				m_tail = b.m_tail;
				b.m_tail = nullptr;
			}
		};

	} // end namespace detail

	// Double linked list.
	template<typename T, typename Pool = CRTMemoryPool>
	class NList : public detail::NListBase<T, detail::NListNode<T>>
	{
	private:
		using Base = detail::NListBase<T, detail::NListNode<T>>;
		using Node = detail::NListNode<T>;

	public:
		using typename Base::Iterator;
		using typename Base::ConstIterator;

		// Default constructor.
		NList()
		{
		}

		// Move.
		NList(NList&& b)
			: Base(std::move(static_cast<Base&>(b)))
		{
		}

		// Copy.
		NList(const NList& b)
		{
			*this = b;
		}

		~NList()
		{
			destroy();
		}

		// Move.
		NList& operator=(NList&& b)
		{
			destroy();
			static_cast<Base&>(*this) = std::move(static_cast<Base&>(b));
			return *this;
		}

		// Copy.
		NList& operator=(const NList& b)
		{
			destroy();
			ConstIterator it = b.getBegin();
			while (it != b.getEnd())
			{
				pushBack(*it);
				++it;
			}

			return *this;
		}

		// Destroy the list.
		void destroy();

		// Copy an element at the end of the list.
		Iterator pushBack(const T& x)
		{
			Node* node = newObject<Node, Pool>(x);
			Base::pushBackNode(node);
			return Iterator(node, this);
		}

		// Construct an element at the end of the list.
		template<typename... TArgs>
		Iterator emplaceBack(TArgs&&... args)
		{
			Node* node = newObject<Node, Pool>(std::forward<TArgs>(args)...);
			Base::pushBackNode(node);
			return Iterator(node, this);
		}

		// Copy an element at the beginning of the list.
		Iterator pushFront(const T& x)
		{
			Node* node = newObject<Node, Pool>(x);
			Base::pushFrontNode(node);
			return Iterator(node, this);
		}

		// Construct element at the beginning of the list.
		template<typename... TArgs>
		Iterator emplaceFront(TArgs&&... args)
		{
			Node* node = newObject<Node, Pool>(std::forward<TArgs>(args)...);
			Base::pushFrontNode(node);
			return Iterator(node, this);
		}

		// Copy an element at the given position of the list.
		Iterator insert(Iterator pos, const T& x)
		{
			Node* node = newObject<Node, Pool>(x);
			Base::insertNode(pos.m_node, node);
			return Iterator(node, this);
		}

		// Construct element at the the given position.
		template<typename... TArgs>
		Iterator emplace(Iterator pos, TArgs&&... args)
		{
			Node* node = newObject<Node, Pool>(std::forward<TArgs>(args)...);
			Base::insertNode(pos.m_node, node);
			return Iterator(node, this);
		}

		// Pop a value from the back of the list.
		void popBack()
		{
			NE_ASSERT(Base::m_tail);
			Node* node = Base::m_tail;
			Base::popBack();
			deletePtr<Node, Pool>(node);
		}

		// Pop a value from the front of the list.
		void popFront()
		{
			NE_ASSERT(Base::m_head);
			Node* node = Base::m_head;
			Base::popFront();
			deletePtr<Node, Pool>(node);
		}

		// Erase an element.
		void erase(Iterator pos)
		{
			NE_ASSERT(pos.m_node);
			NE_ASSERT(pos.m_list == this);
			Base::removeNode(pos.m_node);
			deletePtr(pos.m_node);
		}
	};

	// The classes that will use the IntrusiveNList need to inherit from this one.
	template<typename TClass>
	class IntrusiveNListEnabled
	{
		template<typename, typename, typename, typename>
		friend class detail::NListIterator;

		template<typename, typename>
		friend class detail::NListBase;

		template<typename>
		friend class IntrusiveNList;

		friend TClass;

	public:
		TClass* getPreviousNListNode()
		{
			return m_prev;
		}

		const TClass* getPreviousNListNode() const
		{
			return m_prev;
		}

		TClass* getNextNListNode()
		{
			return m_next;
		}

		const TClass* getNextNListNode() const
		{
			return m_next;
		}

	private:
		TClass* m_prev = nullptr;
		TClass* m_next = nullptr;
	};

	namespace detail {

		// Specialization for IntrusiveNListEnabled
		template<typename TValue>
		class GetNListNodeValueFunc<TValue, TValue>
		{
		public:
			TValue& operator()(TValue& node)
			{
				return node;
			}

			const TValue& operator()(const TValue& node) const
			{
				return node;
			}
		};

	} // end namespace detail

	// NList that doesn't perform any allocations. To work the T nodes will have to inherit from IntrusiveNListEnabled or
	// have 2 member functions and their const versions. The 2 functions are getPreviousNListNode() and getNextNListNode().
	template<typename T>
	class IntrusiveNList : public detail::NListBase<T, T>
	{
		template<typename, typename, typename, typename>
		friend class detail::NListIterator;

	private:
		using Base = detail::NListBase<T, T>;

	public:
		using typename Base::Iterator;

		// Default constructor.
		IntrusiveNList()
			: Base()
		{
		}

		// Move.
		IntrusiveNList(IntrusiveNList&& b)
			: Base(std::move(static_cast<Base&>(b)))
		{
		}

		~IntrusiveNList() = default;

		// Move.
		IntrusiveNList& operator=(IntrusiveNList&& b)
		{
			static_cast<Base&>(*this) = std::move(static_cast<Base&>(b));
			return *this;
		}

		// Copy an element at the end of the list.
		Iterator pushBack(T* x)
		{
			Base::pushBackNode(x);
			return Iterator(x, this);
		}

		// Copy an element at the beginning of the list.
		Iterator pushFront(T* x)
		{
			Base::pushFrontNode(x);
			return Iterator(x, this);
		}

		// Copy an element at the given position of the list.
		Iterator insert(Iterator pos, T* x)
		{
			Base::insertNode(pos.m_node, x);
			return Iterator(x, this);
		}

		// Pop a value from the back of the list.
		T* popBack()
		{
			T* tail = Base::m_tail;
			Base::popBack();
			return tail;
		}

		// Pop a value from the front of the list.
		T* popFront()
		{
			T* head = Base::m_head;
			Base::popFront();
			return head;
		}

		// Erase an element.
		void erase(Iterator pos)
		{
			Base::removeNode(pos.m_node);
		}

		// Erase an element.
		void erase(T* x)
		{
			Base::removeNode(x);
		}
	};
}

#include <nbase/types/List.inl>