#include "base/Logger.hpp"

#include "cal/ast/ASTNodes.hpp"
#include <json/json.h>

namespace cal {

    VariableDeclearNode::VariableDeclearNode(IdentifierNode* name_node, NumType var_type, ASTBase* initial_value)
        : m_name(name_node), m_variable_type(var_type), m_initial_value(initial_value)
    {
        if (name_node->m_type != IdentifierNode::Type::Variable)
        {
            LogError("identifier node is not variable");
            ASSERT(false);
        }

        if (m_initial_value == nullptr)
            return;
        if (var_type == NumType::VariableReturn)
            return;

        if (initial_value->getNumType() != var_type)
        {
            LogError("initial_value's numtype not equal to var_type");
            ASSERT(false);
        }
    }


    Json::Value VariableDeclearNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);

        value["type"] = "Variable";
        value["varName"] = m_name->buildOutput();
        value["varType"] = NumTypeChar[m_variable_type];
        value["initialValue"] = m_initial_value == nullptr ? "nil" : m_initial_value->buildOutput();
        
        return value;
    }


    ASTBase::NodeType VariableDeclearNode::getType() const {
        return NODE_VAR_DECLEAR;
    }


    ASTBase::NumType VariableDeclearNode::getNumType() const {
        return m_variable_type;
    }
}
