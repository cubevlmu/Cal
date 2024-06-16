#include "ASTOPNode.hpp"

#include <json/json.h>

namespace cal {

    static const char* OperatorTypeStr[] = {
        "Add", "Subtract", "Multiply", "Divide", "Modulus",
        "And", "Or", "Not",       
        "Equal", "NotEqual", 
        "LessThan", "GreaterThan",
        "LessOrEqual", "GreaterOrEqual"
    };


    ASTOPNode::ASTOPNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {
    }


    ASTOPNode::~ASTOPNode() = default;


    Json::Value ASTOPNode::buildOutput() const
    {
        Json::Value obj(Json::ValueType::objectValue);

        obj["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        obj["op"] = OperatorTypeStr[(i32)m_type];
        obj["left"] = m_left == nullptr ? "nil" : m_left->buildOutput();
        obj["right"] = m_right == nullptr ? "nil" : m_right->buildOutput();

        return obj;
    }


    ASTTypeNode* ASTOPNode::getReturnType() const
    {
        ASSERT(m_left);
        ASSERT(m_right);

        if (m_left->getReturnType() != m_right->getReturnType())
            return nullptr;
        return m_left->getReturnType();
    }


    void ASTOPNode::set(const OperatorType& type, ASTNodeBase* left, ASTNodeBase* right)
    {
        ASSERT(left);
        ASSERT(right);

        m_left = left;
        m_right = right;
        m_type = type;
    }

}