#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    BlockNode::BlockNode(std::vector<ASTBase*> statements) 
        : m_statements(statements)
    {

    }


    Json::Value BlockNode::buildOutput() const
    {
        Json::Value value(Json::ValueType::objectValue);
        Json::Value contents(Json::ValueType::arrayValue);

        for(auto& node : m_statements) {
            contents.append(node->buildOutput());
        }

        value["type"] = "Block";
        value["content"] = contents;

        return value;
    }
}