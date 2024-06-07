#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    FunctionCallNode::FunctionCallNode(IdentifierNode* functionName, std::vector<ASTBase*> arguments)
        : m_functionName(functionName), m_arguments(arguments)
    {

    }


    Json::Value FunctionCallNode::buildOutput() const
    {
        Json::Value value(Json::ValueType::objectValue);
        Json::Value args(Json::ValueType::arrayValue);
        
        for(auto& arg : m_arguments) {
            args.append(arg->buildOutput());
        }

        value["type"] = "FunctionCall";
        value["name"] = m_functionName->buildOutput();
        value["args"] = args;

        return value;
    }
}