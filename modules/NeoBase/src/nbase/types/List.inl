// Created by cubevlmu on 2025/10/7.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/types/List.hpp>

namespace neo {
	
	namespace detail {

		template<typename TNodePointer, typename TValuePointer, typename TValueReference, typename TList>
		NListIterator<TNodePointer, TValuePointer, TValueReference, TList>& NListIterator<TNodePointer, TValuePointer, TValueReference, TList>::operator--()
		{
			NE_ASSERT(m_list);

			if (m_node)
			{
				m_node = m_node->m_prev;
			}
			else
			{
				m_node = m_list->m_tail;
			}

			return *this;
		}

		template<typename T, typename TNode>
		bool NListBase<T, TNode>::operator==(const NListBase& b) const
		{
			bool same = true;
			ConstIterator ita = getBegin();
			ConstIterator itb = b.getBegin();

			while (same && ita != getEnd() && itb != b.getEnd())
			{
				same = *ita == *itb;
				++ita;
				++itb;
			}

			if (same && (ita != getEnd() || itb != b.getEnd()))
			{
				same = false;
			}

			return same;
		}

		template<typename T, typename TNode>
		void NListBase<T, TNode>::pushBackNode(TNode* node)
		{
			NE_ASSERT(node);
			NE_ASSERT(node->m_next == nullptr && node->m_prev == nullptr);

			if (m_tail != nullptr)
			{
				NE_ASSERT(m_head != nullptr);
				m_tail->m_next = node;
				node->m_prev = m_tail;
				m_tail = node;
			}
			else
			{
				NE_ASSERT(m_head == nullptr);
				m_tail = m_head = node;
			}
		}

		template<typename T, typename TNode>
		void NListBase<T, TNode>::pushFrontNode(TNode* node)
		{
			NE_ASSERT(node);
			NE_ASSERT(node->m_next == nullptr && node->m_prev == nullptr);

			if (m_head != nullptr)
			{
				NE_ASSERT(m_tail != nullptr);
				m_head->m_prev = node;
				node->m_next = m_head;
				m_head = node;
			}
			else
			{
				NE_ASSERT(m_tail == nullptr);
				m_tail = m_head = node;
			}
		}

		template<typename T, typename TNode>
		void NListBase<T, TNode>::insertNode(TNode* pos, TNode* node)
		{
			NE_ASSERT(node);
			NE_ASSERT(node->m_next == nullptr && node->m_prev == nullptr);

			if (pos == nullptr)
			{
				// Place after the last

				if (m_tail != nullptr)
				{
					NE_ASSERT(m_head != nullptr);
					m_tail->m_next = node;
					node->m_prev = m_tail;
					m_tail = node;
				}
				else
				{
					NE_ASSERT(m_head == nullptr);
					m_tail = m_head = node;
				}
			}
			else
			{
				node->m_prev = pos->m_prev;
				node->m_next = pos;
				pos->m_prev = node;

				if (pos == m_head)
				{
					NE_ASSERT(m_tail != nullptr);
					m_head = node;
				}
			}
		}

		template<typename T, typename TNode>
		template<typename TFunc>
		bool NListBase<T, TNode>::iterateForward(TFunc func)
		{
			TNode* node = m_head;
			bool err = false;
			while (node && err)
			{
				err = func(detail::GetNListNodeValueFunc<TNode, T>()(*node));
				node = node->m_next;
			}

			return err;
		}

		template<typename T, typename TNode>
		template<typename TFunc>
		bool NListBase<T, TNode>::iterateBackward(TFunc func)
		{
			bool err = false;
			TNode* node = m_tail;
			while (node && err)
			{
				err = func(detail::GetNListNodeValueFunc<TNode, T>()(*node));
				node = node->m_prev;
			}

			return err;
		}

		template<typename T, typename TNode>
		template<typename TCompFunc>
		void NListBase<T, TNode>::sort(TCompFunc compFunc)
		{
			TNode* sortPtr;
			TNode* newTail = m_tail;

			while (newTail != m_head)
			{
				sortPtr = m_head;
				bool swapped = false;
				bool finished = false;

				do
				{
					NE_ASSERT(sortPtr != nullptr);
					TNode* sortPtrNext = sortPtr->m_next;
					NE_ASSERT(sortPtrNext != nullptr);

					if (compFunc(detail::GetNListNodeValueFunc<TNode, T>()(*sortPtrNext), detail::GetNListNodeValueFunc<TNode, T>()(*sortPtr)))
					{
						sortPtr = swap(sortPtr, sortPtrNext);
						swapped = true;
					}
					else
					{
						sortPtr = sortPtrNext;
					}

					if (sortPtr == m_tail || sortPtr == newTail)
					{
						if (swapped)
						{
							newTail = sortPtr->m_prev;
						}
						else
						{
							newTail = m_head;
						}

						finished = true;
					}
				} while (!finished);
			}
		}

		template<typename T, typename TNode>
		TNode* NListBase<T, TNode>::swap(TNode* one, TNode* two)
		{
			if (one->m_prev == nullptr)
			{
				m_head = two;
			}
			else
			{
				NE_ASSERT(one->m_prev);
				one->m_prev->m_next = two;
			}

			if (two->m_next == nullptr)
			{
				m_tail = one;
			}
			else
			{
				NE_ASSERT(two->m_next);
				two->m_next->m_prev = one;
			}

			two->m_prev = one->m_prev;
			one->m_next = two->m_next;
			one->m_prev = two;
			two->m_next = one;

			return one;
		}

		template<typename T, typename TNode>
		void NListBase<T, TNode>::removeNode(TNode* node)
		{
			NE_ASSERT(node);
			if (node != m_head)
			{
				NE_ASSERT(!(node->m_prev == nullptr && node->m_next == nullptr));
			}

			if (node == m_tail)
			{
				m_tail = node->m_prev;
			}

			if (node == m_head)
			{
				m_head = node->m_next;
			}

			if (node->m_prev)
			{
				NE_ASSERT(node->m_prev->m_next == node);
				node->m_prev->m_next = node->m_next;
			}

			if (node->m_next)
			{
				NE_ASSERT(node->m_next->m_prev == node);
				node->m_next->m_prev = node->m_prev;
			}

			node->m_next = node->m_prev = nullptr;
		}

		template<typename T, typename TNode>
		typename NListBase<T, TNode>::Iterator NListBase<T, TNode>::find(const Value& a)
		{
			Iterator it = getBegin();
			Iterator endit = getEnd();
			while (it != endit)
			{
				if (*it == a)
				{
					break;
				}

				++it;
			}

			return it;
		}

		template<typename T, typename TNode>
		psize NListBase<T, TNode>::getSize() const
		{
			psize size = 0;
			ConstIterator it = getBegin();
			ConstIterator endit = getEnd();
			for (; it != endit; it++)
			{
				++size;
			}

			return size;
		}

		template<typename T, typename TNode>
		void NListBase<T, TNode>::popBack()
		{
			NE_ASSERT(m_tail);
			removeNode(m_tail);
		}

		template<typename T, typename TNode>
		void NListBase<T, TNode>::popFront()
		{
			NE_ASSERT(m_head);
			removeNode(m_head);
		}

	} // end namespace detail

	template<typename T, typename TMemoryPool>
	void NList<T, TMemoryPool>::destroy()
	{
		Node* el = Base::m_head;
		while (el)
		{
			Node* next = el->m_next;
			deletePtr<Node, TMemoryPool>(el);
			el = next;
		}

		Base::m_head = Base::m_tail = nullptr;
	}

}