#include "ASTIdNode.hpp"

#include <json/json.h>

namespace cal {

    static const char* TypeStr[] = {
        "None", "Variable", "FuncName", "ClassName", "Define"
    };


    ASTIdNode::ASTIdNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {
    }


    ASTIdNode::~ASTIdNode() = default;


    Json::Value ASTIdNode::buildOutput() const { 
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["idType"] = TypeStr[(i32)m_type];
        root["idName"] = m_name;

        return root;
    }


    void ASTIdNode::set(const std::string& name, Type type) {
        m_name = name;
        m_type = type;
    }

} // namespace cal