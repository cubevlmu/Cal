#pragma once

#include "analyzer/ast/types/NodeType.hpp"
#include "base/allocator/IAllocator.hpp"
#include "base/types/container/HashMap.hpp"

#include <utils/TSingleton.hpp>

namespace cal {

    class TypePool : public TSingleton<TypePool>
    {
    public:
        TypePool();
        ~TypePool();

        ASTNodeType* getType(const std::string& raw);
        ASTNodeType* getType(ASTNodeType::Types type);

        void releaseType(ASTNodeType* node);

    private:
        IAllocator& m_alloc;
        HashMap<std::string, ASTNodeType*> m_pool;
    };
}