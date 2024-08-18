#include "NodeType.hpp"

#include "TypePool.hpp"
#include "base/allocator/Allocators.hpp"

namespace cal {

    TypePool::TypePool()
        : m_alloc(getGlobalAllocator()),
        m_pool(getGlobalAllocator())
    {

    }


    TypePool::~TypePool() {
        for (auto* item : m_pool) {
            CAL_DEL(m_alloc, item);
        }
        m_pool.clear();
    }


    ASTNodeType* TypePool::getType(const std::string& raw) {
        auto result = m_pool.find(raw);
        if (result.isValid()) {
            return result.value();
        }

        auto* ptr = CAL_NEW(m_alloc, ASTNodeType)(m_alloc, raw);
        bool rr = ptr->parse(raw);
        if (!rr) {
            ASTWarn("pool : failed to parse an type -> ", raw);
            CAL_DEL(m_alloc, ptr);
            return nullptr;
        }

        m_pool.insert(raw, ptr);
        return ptr;
    }


    ASTNodeType* TypePool::getType(ASTNodeType::Types type) {
        if(type == ASTNodeType::Types::custom || type == ASTNodeType::Types::unknown) {
            ASTWarn("pool : unsupport type!");
            return nullptr;
        }
        std::string typeStr = ASTNodeType::typeToStr(type);
        return getType(typeStr);
    }


    void TypePool::releaseType(ASTNodeType *node) {
        auto result = m_pool.find(node->getRawTypeName());
        if (!result.isValid()) {
            return;
        }

        m_pool.erase(result);
        CAL_DEL(m_alloc, node);
    }

} // namespace cal