#include "cal/ast/ASTNodeBase.hpp"

#include "base/Utils.hpp"
#include "ASTNodeBase.hpp"

namespace cal {

    const char* ASTNodeBase::ASTNodeTypesString[] = {
        "AST_OP", "AST_NUMBER", "AST_TEXT", "AST_IDENTIFIER",
        "AST_BLOCK", "AST_TYPE", "AST_FUNC_DEF", "AST_FUNC_ARG_DEF",
        "AST_VAR", "AST_ASSIGNMENT", "AST_FUNC_CALL", "AST_FUNC_RET"
    };


    ASTNodeBase::ASTNodeBase(IAllocator& alloc)
        : m_allocator(alloc)
    {
    }


    ASTNodeBase::~ASTNodeBase() = default;
    
    
    ASTTypeNode* ASTNodeBase::getReturnType() const
    {
        return nullptr;
    }


    std::string ASTNodeBase::toString() const
    {
        BeginAppender();
        AppenderAppend("[Node: ");
        AppenderAppend(ASTNodeBase::ASTNodeTypesString[(i32)getType()]);
        AppenderAppend("]");
        return EndAppender();
    }

}