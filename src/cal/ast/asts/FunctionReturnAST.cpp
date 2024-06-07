#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    FunctionReturnNode::FunctionReturnNode(ASTBase* returnValue) {
        m_returnValue = returnValue;
        if(returnValue == nullptr) {
            m_voidReturn = true;
        }
    }


    Json::Value FunctionReturnNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);

        value["type"] = "FunctionReturn";
        value["body"] = m_voidReturn ? "[void]" : m_returnValue->buildOutput();
        
        return value;
    }
}