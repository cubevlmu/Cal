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