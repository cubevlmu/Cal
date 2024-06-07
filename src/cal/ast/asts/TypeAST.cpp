#include "cal/ast/ASTNodes.hpp"

#include <json/json.h>

namespace cal {

    TypeNode::TypeNode(const std::string& node_str)
        : m_raw_type(node_str)
    {
        if (m_raw_type.find("[]") > 0) {
            m_isArray = true;
            m_raw_type.replace(m_raw_type.find("[]"), 2, "");
        }

        NumType type = parseNumTypeText(m_raw_type);
        if (type == NumType::ERR_TP)
            type = NumType::USER_DEFINED;
        m_type = type;
    }


    Json::Value TypeNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);

        value["type"] = "Type";
        value["numType"] = NumTypeChar[m_type];
        value["rawType"] = m_raw_type;
        value["isArray"] = m_isArray ? "yes" : "no";

        return value;
    }


    ASTBase::NodeType TypeNode::getType() const {
        return NODE_TYPE;
    }


    ASTBase::NumType TypeNode::getNumType() const {
        return m_type;
    }
}