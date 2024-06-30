#include "ASTFuncReturnNode.hpp"

#include <json/json.h>
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"

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


    bool ASTFuncReturnNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        bool cr = m_val->checkSyntax(analyzer);
        if (!cr) return cr;

        if (analyzer->m_now_func_state == nullptr) {
            analyzer->m_pc->addError("a return operator found in non function scope!", analyzer);
            return false;
        }

        if (!analyzer->m_now_func_state->func_declear.m_ret_type->compareType(m_val->getReturnType())) {
            analyzer->m_pc->addError("return value is not equal to function's decleared return type!", analyzer);
            return false;
        }

        return true;
    }
}