#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    const char* OpTypeMsg[] = {
        "*",
        "/",
        "+",
        "-"
    };


    OpNode::OpNode(OpType type, ASTBase* left, ASTBase* right)
        : m_op_type(type), m_left(left), m_right(right)
    {

    }


    Json::Value OpNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);

        value["type"] = "Operator";
        value["op"] = OpTypeMsg[m_op_type];
        value["left"] = m_left->buildOutput();
        value["right"] = m_right->buildOutput();

        return value;
    }
}
