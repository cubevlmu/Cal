#include "base/Utils.hpp"

#include "cal/ast/ASTNodes.hpp"

namespace cal {

    BlockNode::BlockNode(std::vector<ASTBase*> statements) 
        : m_statements(statements)
    {

    }


    std::string BlockNode::toString() const
    {
        BeginAppender();
        AppenderAppend("[Block] \n");
        for(auto& node : m_statements) {
            AppenderAppend("\t");
            AppenderAppend(node->toString());
            AppenderAppend("\n");
        }
        return EndAppender();
    }
}