#pragma once

#include "globals.hpp"
#include "base/allocator/IAllocator.hpp"
#include "base/allocator/TagAllocator.hpp"
#include <utility>

namespace cal {

    namespace detail {

        /// List node.
        /// @internal
        template<typename T>
        class ListNode
        {
        public:
            T m_value;
            ListNode* m_prev = nullptr;
            ListNode* m_next = nullptr;

            template<typename... TArgs>
            ListNode(TArgs&&... args)
                : m_value(std::forward<TArgs>(args)...)
            {
            }
        };

        /// Gets the value of a list node.
        /// @internal
        template<typename TNode, typename TValue>
        class GetListNodeValueFunc
        {
        public:
            TValue& operator()(TNode& node);
            const TValue& operator()(const TNode& node) const;
        };

        /// Specialization for ListNode
        /// @internal
        template<typename TValue>
        class GetListNodeValueFunc<ListNode<TValue>, TValue>
        {
        public:
            TValue& operator()(ListNode<TValue>& node)
            {
                return node.m_value;
            }

            const TValue& operator()(const ListNode<TValue>& node) const
            {
                return node.m_value;
            }
        };

        /// List bidirectional iterator.
        /// @internal
        template<typename TNodePointer, typename TValuePointer, typename TValueReference, typename TListPointer>
        class ListIterator
        {
            template<typename, typename>
            friend class ListBase;

            template<typename, typename>
            friend class List;

            template<typename, typename, typename, typename>
            friend class ListIterator;

        public:
            ListIterator() = default;

            ListIterator(const ListIterator& b)
                : m_node(b.m_node)
                , m_list(b.m_list)
            {
            }

            /// Allow conversion from iterator to const iterator.
            template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YList>
            ListIterator(const ListIterator<YNodePointer, YValuePointer, YValueReference, YList>& b)
                : m_node(b.m_node)
                , m_list(b.m_list)
            {
            }

            ListIterator(TNodePointer node, TListPointer list)
                : m_node(node)
                , m_list(list)
            {
                ASSERT(list);
            }

            ListIterator& operator=(const ListIterator& b)
            {
                m_node = b.m_node;
                m_list = b.m_list;
                return *this;
            }

            TValueReference operator*() const
            {
                ASSERT(m_node);
                using NodeType = typename RemovePointer<TNodePointer>::Type;
                using ValueType = typename RemovePointer<TValuePointer>::Type;
                return detail::GetListNodeValueFunc<NodeType, ValueType>()(*m_node);
            }

            TValuePointer operator->() const
            {
                ASSERT(m_node);
                using NodeType = typename RemovePointer<TNodePointer>::Type;
                using ValueType = typename RemovePointer<TValuePointer>::Type;
                return &detail::GetListNodeValueFunc<NodeType, ValueType>()(*m_node);
            }

            ListIterator& operator++()
            {
                ASSERT(m_node);
                m_node = m_node->m_next;
                return *this;
            }

            ListIterator operator++(int)
            {
                ASSERT(m_node);
                ListIterator out = *this;
                ++(*this);
                return out;
            }

            ListIterator& operator--();

            ListIterator operator--(int)
            {
                ASSERT(m_node);
                ListIterator out = *this;
                --(*this);
                return out;
            }

            ListIterator operator+(u32 n) const
            {
                ListIterator it = *this;
                while (n-- != 0)
                {
                    ++it;
                }
                return it;
            }

            ListIterator operator-(u32 n) const
            {
                ListIterator it = *this;
                while (n-- != 0)
                {
                    --it;
                }
                return it;
            }

            ListIterator& operator+=(u32 n)
            {
                while (n-- != 0)
                {
                    ++(*this);
                }
                return *this;
            }

            ListIterator& operator-=(u32 n)
            {
                while (n-- != 0)
                {
                    --(*this);
                }
                return *this;
            }

            template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YList>
            bool operator==(const ListIterator<YNodePointer, YValuePointer, YValueReference, YList>& b) const
            {
                ASSERT(m_list == b.m_list);
                return m_node == b.m_node;
            }

            template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YList>
            bool operator!=(const ListIterator<YNodePointer, YValuePointer, YValueReference, YList>& b) const
            {
                return !(*this == b);
            }

        private:
            TNodePointer m_node = nullptr;
            TListPointer m_list = nullptr; ///< Used to go back from the end
        };

        /// Double linked list base.
        /// @internal
        template<typename T, typename TNode>
        class ListBase
        {
            template<typename, typename, typename, typename>
            friend class ListIterator;

        public:
            using Value = T;
            using Reference = Value&;
            using ConstReference = const Value&;
            using Pointer = Value*;
            using ConstPointer = const Value*;
            using Iterator = ListIterator<TNode*, Pointer, Reference, ListBase*>;
            using ConstIterator = ListIterator<const TNode*, ConstPointer, ConstReference, const ListBase*>;

            ListBase() = default;

            ListBase(ListBase&& b)
            {
                move(b);
            }

            ListBase(const ListBase&) = delete; // Non-copyable

            ListBase& operator=(const ListBase&) = delete; // Non-copyable

            ListBase& operator=(ListBase&& b)
            {
                move(b);
                return *this;
            }

            /// Compare with another list.
            bool operator==(const ListBase& b) const;

            /// Get first element.
            ConstReference getFront() const
            {
                ASSERT(!isEmpty());
                return detail::GetListNodeValueFunc<TNode, T>()(*m_head);
            }

            /// Get first element.
            Reference getFront()
            {
                ASSERT(!isEmpty());
                return detail::GetListNodeValueFunc<TNode, T>()(*m_head);
            }

            /// Get last element.
            ConstReference getBack() const
            {
                ASSERT(!isEmpty());
                return detail::GetListNodeValueFunc<TNode, T>()(*m_tail);
            }

            /// Get last element.
            Reference getBack()
            {
                ASSERT(!isEmpty());
                return detail::GetListNodeValueFunc<TNode, T>()(*m_tail);
            }

            /// Get begin.
            Iterator getBegin()
            {
                return Iterator(m_head, this);
            }

            /// Get begin.
            ConstIterator getBegin() const
            {
                return ConstIterator(m_head, this);
            }

            /// Get end.
            Iterator getEnd()
            {
                return Iterator(nullptr, this);
            }

            /// Get end.
            ConstIterator getEnd() const
            {
                return ConstIterator(nullptr, this);
            }

            /// Get begin.
            Iterator begin()
            {
                return getBegin();
            }

            /// Get begin.
            ConstIterator begin() const
            {
                return getBegin();
            }

            /// Get end.
            Iterator end()
            {
                return getEnd();
            }

            /// Get end.
            ConstIterator end() const
            {
                return getEnd();
            }

            /// Return true if list is empty.
            bool isEmpty() const
            {
                return m_head == nullptr;
            }

            /// Iterate the list using lambda.
            template<typename TFunc>
            Error iterateForward(TFunc func);

            /// Iterate the list backwards using lambda.
            template<typename TFunc>
            Error iterateBackward(TFunc func);

            /// Find item.
            Iterator find(const Value& a);

            /// Sort the list.
            /// @note It's almost 300 slower than std::list::sort, at some point replace the algorithm.
            template<typename TCompFunc = std::less<Value>>
            void sort(TCompFunc compFunc = TCompFunc());

            /// Compute the size of elements in the list.
            ptr getSize() const;

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
            /// Used in sort.
            TNode* swap(TNode* one, TNode* two);

            void move(ListBase& b)
            {
                m_head = b.m_head;
                b.m_head = nullptr;
                m_tail = b.m_tail;
                b.m_tail = nullptr;
            }
        };

    } // end namespace detail

    /// Double linked list.
    template<typename T>
    class List : public detail::ListBase<T, detail::ListNode<T>>
    {
    private:
        using Base = detail::ListBase<T, detail::ListNode<T>>;
        using Node = detail::ListNode<T>;

    public:
        using typename Base::Iterator;
        using typename Base::ConstIterator;

        /// Default constructor.
        List(IAllocator& alloc)
            : m_alloc(TagAllocator(alloc, "ToyList"))
        {
        }

        /// Move.
        List(List&& b)
            : Base(std::move(static_cast<Base&>(b)))
            , m_alloc(std::move(b))
        {
        }

        /// Copy.
        List(const List& b)
        {
            *this = b;
        }

        ~List()
        {
            destroy();
        }

        /// Move.
        List& operator=(List&& b)
        {
            destroy();
            static_cast<Base&>(*this) = std::move(static_cast<Base&>(b));
            m_alloc = std::move(b.m_alloc);
            return *this;
        }

        /// Copy.
        List& operator=(const List& b)
        {
            destroy();
            for (ConstIterator it : b)
            {
                pushBack(*it);
            }

            return *this;
        }

        /// Destroy the list.
        void destroy();

        /// Copy an element at the end of the list.
        Iterator pushBack(const T& x)
        {
            Node* node = CAL_NEW(m_alloc, Node)(x);
            Base::pushBackNode(node);
            return Iterator(node, this);
        }

        /// Construct an element at the end of the list.
        template<typename... TArgs>
        Iterator emplaceBack(TArgs&&... args)
        {
            Node* node = CAL_NEW(m_alloc, Node)(std::forward<TArgs>(args)...);
            Base::pushBackNode(node);
            return Iterator(node, this);
        }

        /// Copy an element at the beginning of the list.
        Iterator pushFront(const T& x)
        {
            Node* node = CAL_NEW(m_alloc, Node)(x);
            Base::pushFrontNode(node);
            return Iterator(node, this);
        }

        /// Construct element at the beginning of the list.
        template<typename... TArgs>
        Iterator emplaceFront(TArgs&&... args)
        {
            Node* node = CAL_NEW(m_alloc, Node)(std::forward<TArgs>(args)...);
            Base::pushFrontNode(node);
            return Iterator(node, this);
        }

        /// Copy an element at the given position of the list.
        Iterator insert(Iterator pos, const T& x)
        {
            Node* node = CAL_NEW(m_alloc, Node)(x);
            Base::insertNode(pos.m_node, node);
            return Iterator(node, this);
        }

        /// Construct element at the the given position.
        template<typename... TArgs>
        Iterator emplace(Iterator pos, TArgs&&... args)
        {
            Node* node = CAL_NEW(m_alloc, Node)(std::forward<TArgs>(args)...);
            Base::insertNode(pos.m_node, node);
            return Iterator(node, this);
        }

        /// Pop a value from the back of the list.
        void popBack()
        {
            ASSERT(Base::m_tail);
            Node* node = Base::m_tail;
            Base::popBack();
            deleteInstance(m_alloc, node);
        }

        /// Pop a value from the front of the list.
        void popFront()
        {
            ASSERT(Base::m_head);
            Node* node = Base::m_head;
            Base::popFront();
            deleteInstance(m_alloc, node);
        }

        /// Erase an element.
        void erase(Iterator pos)
        {
            ASSERT(pos.m_node);
            ASSERT(pos.m_list == this);
            Base::removeNode(pos.m_node);
            deleteInstance(m_alloc, pos.m_node);
        }

        IAllocator& getAllocator()
        {
            return m_alloc;
        }

    private:
        TagAllocator m_alloc;
    };

    /// The classes that will use the IntrusiveList need to inherit from this one.
    template<typename TClass>
    class IntrusiveListEnabled
    {
        template<typename, typename, typename, typename>
        friend class detail::ListIterator;

        template<typename, typename>
        friend class detail::ListBase;

        template<typename>
        friend class IntrusiveList;

        friend TClass;

    public:
        TClass* getPreviousListNode()
        {
            return m_prev;
        }

        const TClass* getPreviousListNode() const
        {
            return m_prev;
        }

        TClass* getNextListNode()
        {
            return m_next;
        }

        const TClass* getNextListNode() const
        {
            return m_next;
        }

    private:
        TClass* m_prev = nullptr;
        TClass* m_next = nullptr;
    };

    namespace detail {

        /// Specialization for IntrusiveListEnabled
        /// @internal
        template<typename TValue>
        class GetListNodeValueFunc<TValue, TValue>
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

    /// List that doesn't perform any allocations. To work the T nodes will have to inherit from IntrusiveListEnabled or
    /// have 2 member functions and their const versions. The 2 functions are getPreviousListNode() and getNextListNode().
    template<typename T>
    class IntrusiveList : public detail::ListBase<T, T>
    {
        template<typename, typename, typename, typename>
        friend class detail::ListIterator;

    private:
        using Base = detail::ListBase<T, T>;

    public:
        using typename Base::Iterator;

        /// Default constructor.
        IntrusiveList()
            : Base()
        {
        }

        /// Move.
        IntrusiveList(IntrusiveList&& b)
            : Base(std::move(static_cast<Base&>(b)))
        {
        }

        ~IntrusiveList() = default;

        /// Move.
        IntrusiveList& operator=(IntrusiveList&& b)
        {
            static_cast<Base&>(*this) = std::move(static_cast<Base&>(b));
            return *this;
        }

        /// Copy an element at the end of the list.
        Iterator pushBack(T* x)
        {
            Base::pushBackNode(x);
            return Iterator(x, this);
        }

        /// Copy an element at the beginning of the list.
        Iterator pushFront(T* x)
        {
            Base::pushFrontNode(x);
            return Iterator(x, this);
        }

        /// Copy an element at the given position of the list.
        Iterator insert(Iterator pos, T* x)
        {
            Base::insertNode(pos.m_node, x);
            return Iterator(x, this);
        }

        /// Pop a value from the back of the list.
        T* popBack()
        {
            T* tail = Base::m_tail;
            Base::popBack();
            return tail;
        }

        /// Pop a value from the front of the list.
        T* popFront()
        {
            T* head = Base::m_head;
            Base::popFront();
            return head;
        }

        /// Erase an element.
        void erase(Iterator pos)
        {
            Base::removeNode(pos.m_node);
        }

        /// Erase an element.
        void erase(T* x)
        {
            Base::removeNode(x);
        }
    };
}

#include "List.inl.h"