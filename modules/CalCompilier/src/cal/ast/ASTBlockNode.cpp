#include "ASTBlockNode.hpp"

#include "ASTTypeNode.hpp"
#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"

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

        for (auto* item : m_nodes) {
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
        node->m_parent = this;
        m_nodes.push(node);
    }


    Array<ASTNodeBase*> ASTBlockNode::pick(SortType type) const
    {
        Array<ASTNodeBase*> rst(m_allocator);

        for (ASTNodeBase* node : m_nodes) {
            if (node->getType() == ASTNodeBase::ASTNodeTypes::AST_FUNC_DEF && type == SortType::FUNC_DECLEAR) {
                rst.push(node);
            } else if (node->getType() == ASTNodeBase::ASTNodeTypes::AST_VAR && type == SortType::VAR_DECLEAR) {
                rst.push(node);
            } else {

            }
        }

        return rst;
    }


    void ASTBlockNode::setFuncReturnType(ASTTypeNode* type)
    {

    }


    bool ASTBlockNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        // so if any node has syntax error, block node's check function will return false to parent level
        bool cr = true;
        for (auto* node : m_nodes) {
            cr |= node->checkSyntax(analyzer);
        }
        return cr;
    }
}