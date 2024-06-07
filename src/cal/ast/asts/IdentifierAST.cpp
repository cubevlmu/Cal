#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    const char* IDF_Type_Name[] = {
        "Variable ",
        "FunctionName ",
        "FuncCallArg "
    };


    IdentifierNode::IdentifierNode(Type type, const std::string& name)
        : m_type(type), m_name(name)
    { }


    Json::Value IdentifierNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);
        
        value["type"] = "Identifier";
        value["body"] = IDF_Type_Name[m_type];
        value["name"] = m_name;
        
        return value;
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