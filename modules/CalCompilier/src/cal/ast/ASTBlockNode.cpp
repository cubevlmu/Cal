#include "ASTBlockNode.hpp"

#include "ASTTypeNode.hpp"
#include <json/json.h>

namespace cal {

    ASTBlockNode::ASTBlockNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_nodes(alloc)
    {

    }


    ASTBlockNode::~ASTBlockNode() {
        m_nodes.clear();
    }


    Json::Value ASTBlockNode::buildOutput() const
    {
        Json::Value root(Json::ValueType::objectValue);
        Json::Value node(Json::ValueType::arrayValue);

        for(auto* item : m_nodes) {
            node.append(item->buildOutput());
        }

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["node"] = node;

        return root;
    }


    ASTTypeNode* ASTBlockNode::getReturnType() const
    {
        return nullptr;
    }


    void ASTBlockNode::addNode(ASTNodeBase* node)
    {
        m_nodes.push(node);
    }


    void ASTBlockNode::setFuncReturnType(ASTTypeNode* type)
    {

    }
}