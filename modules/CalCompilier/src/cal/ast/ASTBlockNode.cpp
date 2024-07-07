#include "ASTBlockNode.hpp"

#include "ASTTypeNode.hpp"
#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

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


    std::string ASTBlockNode::toString() const
    {
        StringBuilder nodes {};
        for (auto* item : m_nodes) {
            nodes.append(item->toString());
        }

        return StringBuilder {
            ASTNodeBase::toString(),
            " [Type:", ASTNodeBase::ASTNodeTypesString[(i32)getType()], "]",
            " [Nodes(", m_nodes.size(), "):{", nodes, "}]",
        };
    }


    ASTTypeNode* ASTBlockNode::getReturnType() const
    {
        return nullptr;
    }


    void ASTBlockNode::addNode(ASTNodeBase* node)
    {
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
            } else if (node->getType() == ASTNodeBase::ASTNodeTypes::AST_STRUCT && type == SortType::STRUCT_DECLEAR) {
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