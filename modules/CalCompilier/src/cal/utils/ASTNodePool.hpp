#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/allocator/TagAllocator.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include <string>

namespace cal {

    class ASTPoolItem
    {
    public:
        virtual std::string key() const = 0;
    };


    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ASTNodeBase, T> && std::is_base_of_v<ASTPoolItem, T>>>
    class ASTNodePool
    {
    public:
        ASTNodePool(IAllocator& alloc, const std::string& name = "SimplePool")
            : m_allocator(alloc, name.c_str()), m_nodes(m_allocator)
        { }


        ~ASTNodePool() {
            for (T* node : m_nodes) {
                CAL_DEL(m_allocator, node);
            }
            m_nodes.clear();
        }


        T* getFromPool(const std::string& key) {
            u32 hash = static_cast<u32>(std::hash<std::string>{}(key));
            if (!m_nodes.find(hash).isValid()) {
                return nullptr;
            }

            return m_nodes[hash];
        }


        T* newToPool(const std::string& key) {
            auto* rs = getFromPool(key);
            if (rs != nullptr) return rs;

            auto* node = CAL_NEW(m_allocator, T)(m_allocator);

            u32 hash = static_cast<u32>(std::hash<std::string>{}(key));
            m_nodes.insert(hash, node);

            return node;
        }


        void clearPool() {
            for (auto* ptr : m_nodes) {
                CAL_DEL(m_allocator, ptr);
            }
            m_nodes.clear();
        }

    private:
        HashMap<u32, T*> m_nodes;
        TagAllocator m_allocator;
    };
}