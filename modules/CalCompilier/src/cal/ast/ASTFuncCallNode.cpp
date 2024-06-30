#include "ASTFuncCallNode.hpp"

#include <json/json.h>
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"

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
        auto fidx = analyzer->m_funcs.find(m_name);
        if (!fidx.isValid()) {
            analyzer->m_pc->addError("failed to found function", analyzer);
            return false;
        }

        SyntaxAnalyzer::FuncDetail& dtl = fidx.value();
        if (dtl.m_types.size() != m_args.size()) {
            analyzer->m_pc->addError("function's declear's argument count can't fit with current arguments provided", analyzer);
            return false;
        }

        bool cr = true;
        for(auto i = 0; i < m_args.size(); i++) {
            auto* arg = m_args[i];

            cr |= arg->checkSyntax(analyzer);
            cr |= dtl.m_types[i]->compareType(arg->getReturnType());
        }
        if(!cr) {
            analyzer->m_pc->addError("failed to resolving function calling args, it's not fit with the declear", analyzer);
            return cr;
        }

        m_ret_type = dtl.m_ret_type;
        return true;
    }

} // namespace cal