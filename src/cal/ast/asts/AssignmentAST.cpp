#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    AssignmentNode::AssignmentNode(IdentifierNode* variable, ASTBase* value)
        : m_variable(variable), m_value(value)
    { }


    Json::Value AssignmentNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);
        
        value["type"]   = "Assignment";
        value["result"] = m_variable->buildOutput();
        value["assign"] = m_value->buildOutput();  
        
        return value;
    }
}