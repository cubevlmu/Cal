#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    FunctionNode::FunctionNode(const std::string& name, std::vector<FunctionArgNode*> args, NumType returnType, BlockNode* content)
        : m_name(name), m_args(args), m_returnType(returnType), m_func_content(content)
    {

    }


    Json::Value FunctionNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);
        Json::Value func_args(Json::ValueType::arrayValue);

        for (auto* arg : m_args) {
            func_args.append(arg->buildOutput());
        }

        value["type"] = "Function";
        value["name"] = m_name;
        value["args"] = func_args;
        value["returnType"] = NumTypeChar[m_returnType];
        value["body"] = m_func_content == nullptr ? "Empty" : m_func_content->buildOutput();

        return value;
    }


    ASTBase::NumType FunctionNode::getNumType() const {
        return m_returnType;
    }
}