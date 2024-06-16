#include "ASTAssignmentNode.hpp"

#include <json/json.h>

namespace cal {

    ASTAssignmentNode::ASTAssignmentNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {
    }


    Json::Value ASTAssignmentNode::buildOutput() const
    {
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["assignName"] = m_var_name;
        root["assignsVal"] = m_value->buildOutput();

        return root;
    }


    void ASTAssignmentNode::set(const std::string& name, ASTNodeBase* value)
    {
        m_var_name = name;
        if(value == nullptr) {
            throw std::runtime_error("assigning value but value node is missing");
        }

        m_value = value;
    }

}