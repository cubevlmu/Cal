#pragma once

#include "analyzer/ast/NodeBase.hpp"
#include "analyzer/ast/types/NodeType.hpp"
#include "base/allocator/IAllocator.hpp"

namespace cal {

    class ASTExprNode : public ASTNodeBase 
    {
    public:
        ASTExprNode(IAllocator& alloc) : ASTNodeBase(alloc) {}
        virtual ASTNodeType* returnType() = 0;
    };
}