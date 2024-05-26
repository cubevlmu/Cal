#include "base/Utils.hpp"
#include "cal/ast/ASTNodes.hpp"

namespace cal {

    const char* OpTypeMsg[] = {
        "*",
        "/",
        "+",
        "-"
    };


    OpNode::OpNode(OpType type, ASTBase* left, ASTBase* right)
        : m_op_type(type), m_left(left), m_right(right)
    {

    }


    std::string cal::OpNode::toString() const { 
        BeginAppender();
        AppenderAppend("[OpNode] \n\t");
        AppenderAppend(m_left->toString());
        AppenderAppend(" \n\t");
        AppenderAppend(OpTypeMsg[m_op_type]);
        AppenderAppend("\n\t");
        AppenderAppend(m_right->toString());
        return EndAppender();
    }
}

