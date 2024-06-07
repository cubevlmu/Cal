#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    FunctionArgNode::FunctionArgNode(const std::string& name, NumType type) 
        : m_name(name), m_type(type)
    {

    }


    Json::Value FunctionArgNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);

        value["type"] = "FunctionArgument";
        value["argName"] = m_name;
        value["argType"] = NumTypeChar[m_type];

        return value;
    }


    ASTBase::NumType FunctionArgNode::getNumType() const {
        return m_type; //TODO
    }
}