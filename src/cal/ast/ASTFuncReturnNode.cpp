#include "ASTFuncReturnNode.hpp"

#include <json/json.h>

namespace cal {

    ASTFuncReturnNode::ASTFuncReturnNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {
    }


    ASTFuncReturnNode::~ASTFuncReturnNode() = default;


    Json::Value ASTFuncReturnNode::buildOutput() const {
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["returnType"] = (i32)m_type;
        root["returnValue"] = m_val->buildOutput();

        return root;
    }


    void ASTFuncReturnNode::set(ASTNodeBase* val, ReturnType type)
    {
        ASSERT(val);
        m_val = val;
        m_type = type;
    }
}