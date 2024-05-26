#include "cal/ast/ASTNodes.hpp"

#include "base/Utils.hpp"

namespace cal {

    const char* IDF_Type_Name[] = {
        "Variable ",
        "FunctionName ",
        "FuncCallArg "
    };


    IdentifierNode::IdentifierNode(Type type, const std::string& name)
        : m_type(type), m_name(name)
    { }


    std::string IdentifierNode::toString() const {
        BeginAppender();
        AppenderAppend("[Identifier] \n\tType: ");
        AppenderAppend(IDF_Type_Name[m_type]);
        AppenderAppend(" \n\t");
        AppenderAppend(m_name);
        return EndAppender();
    }


    ASTBase::NumType IdentifierNode::getNumType() const
    {
        switch (m_type) {
        case FuncCallArg:
        case Variable:
            return ASTBase::NumType::VariableReturn;
            break;
        case Function:
        default:
            return ASTBase::NumType::ERR_TP;
            break;
        };
    }
}


