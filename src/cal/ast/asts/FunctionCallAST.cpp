#include "base/Utils.hpp"
#include "cal/ast/ASTNodes.hpp"

namespace cal {

    FunctionCallNode::FunctionCallNode(IdentifierNode* functionName, std::vector<ASTBase*> arguments)
        : m_functionName(functionName), m_arguments(arguments)
    {

    }


    std::string FunctionCallNode::toString() const
    {
        BeginAppender();
        AppenderAppend("[FunctionCall] \n\tName: \n\t");
        AppenderAppend(m_functionName->toString());
        AppenderAppend("\n\tArguments: ");
        for(auto& arg : m_arguments) {
            AppenderAppend(arg->toString());
            AppenderAppend("\n\t");
        }

        return EndAppender();
    }
}