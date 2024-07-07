#include "ASTFuncCallNode.hpp"

#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

namespace cal {

    ASTFuncCallNode::ASTFuncCallNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_args(alloc)
    {
    }


    ASTFuncCallNode::~ASTFuncCallNode() = default;


    Json::Value ASTFuncCallNode::buildOutput() const {
        Json::Value root(Json::ValueType::objectValue);
        Json::Value args(Json::ValueType::arrayValue);
        for (auto* arg : m_args) {
            args.append(arg->buildOutput());
        }

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["funcName"] = m_name;
        root["funcArgs"] = args;

        return root;
    }


    std::string ASTFuncCallNode::toString() const
    {
        StringBuilder args;
        for (auto* arg : m_args) {
            args.append(arg->toString());
        }

        return StringBuilder {
            ASTNodeBase::toString(),
            " [Name:", m_name, "]",
            " [Args(", m_args.size(), "):{", args, "}]"
        };
    }


    void ASTFuncCallNode::setName(const std::string& name)
    {
        m_name = name;
    }


    void ASTFuncCallNode::addParam(ASTNodeBase* node) {
        if (node == nullptr)
            throw std::runtime_error("arg node can't be null");
        m_args.push(node);
    }


    void ASTFuncCallNode::delParam(ASTNodeBase* node) {
        if (node == nullptr)
            return;
        // if (m_args.find(node) == -1)
        //     return;
        m_args.eraseItem(node);
    }


    void ASTFuncCallNode::clearParams() {
        m_args.clear();
    }


    bool ASTFuncCallNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        auto dtl = analyzer->m_table->getFunction(m_name);
        if (dtl == SymbolTable::INVALID_FUNC) {
            analyzer->m_pc->addError(S("Failed to resolve function: ", m_name), analyzer);
            return false;
        }

        if (dtl.m_types.size() != m_args.size()) {
            analyzer->m_pc->addError(S("Function's declearation argument count can't fit with current arguments provided. At ", m_name), analyzer);
            return false;
        }

        bool cr = true;
        for(auto i = 0; i < m_args.size(); i++) {
            auto* arg = m_args[i];

            cr |= arg->checkSyntax(analyzer);
            cr |= dtl.m_types[i]->compareType(arg->getReturnType());
        }
        if(!cr) {
            analyzer->m_pc->addError(S("Failed to resolving function calling args, it's not fit with the declear. At ", m_name), analyzer);
            return cr;
        }

        m_ret_type = dtl.m_ret_type;
        return true;
    }

} // namespace cal