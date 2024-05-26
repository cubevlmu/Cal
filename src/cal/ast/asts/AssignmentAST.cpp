#include "base/Utils.hpp"
#include "cal/ast/ASTNodes.hpp"

namespace cal {

    AssignmentNode::AssignmentNode(IdentifierNode* variable, ASTBase* value)
        : m_variable(variable), m_value(value)
    { }


    std::string AssignmentNode::toString() const {
        BeginAppender();
        AppenderAppend("[Assignment] Result:\n\t");
        AppenderAppend(m_variable->toString());
        AppenderAppend("\n\t= \n\t");
        AppenderAppend(m_value->toString());
        return EndAppender();
    }
}