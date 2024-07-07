#include "ASTVarDefNode.hpp"

#include "ASTTypeNode.hpp"
#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

namespace cal {

    ASTVarDefNode::ASTVarDefNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {

    }


    ASTVarDefNode::~ASTVarDefNode() = default;


    Json::Value ASTVarDefNode::buildOutput() const {
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["varName"] = m_name;
        root["varType"] = m_type->buildOutput();
        root["varInitialValue"] = m_initial_value == nullptr ? "null" : m_initial_value->buildOutput();

        return root;
    }


    std::string ASTVarDefNode::toString() const
    {
        return StringBuilder { 
            ASTNodeBase::toString(), 
            " [Name:", m_name, "]",
            " [Type:", m_type->toString(), "]",
            " [IntialValue:", m_initial_value == nullptr ? "" : m_initial_value->toString(), "]"
        };
    }


    void ASTVarDefNode::set(
        const std::string& name,
        ASTTypeNode* node,
        ASTNodeBase* initial_value
    ) {
        m_name = name;

        if (node == nullptr && initial_value == nullptr) {
            throw new std::runtime_error("variable type not defined (tips: maybe you can give a initial value for it)");
        }

        if (node == nullptr) {
            m_type = initial_value->getReturnType();
        }
        else {
            m_type = node;
        }

        m_initial_value = initial_value;
        if (m_initial_value == nullptr)
            return;

        // if (!m_initial_value->getReturnType()->compareType(m_type))
        //     throw std::runtime_error("type not fit for the declear type and initial value's type");
    }


    void ASTVarDefNode::set(
        const std::string& name,
        ASTNodeBase* initial_value
    ) {
        m_name = name;

        if (initial_value == nullptr) {
            throw std::runtime_error("variable type not defined (tips: maybe you can give a initial value for it)");
        }

        m_type = initial_value->getReturnType();
        m_initial_value = initial_value;

        if (!m_initial_value->getReturnType()->compareType(m_type))
            throw std::runtime_error("type not fit for the declear type and initial value's type");
    }


    bool ASTVarDefNode::compareType(ASTTypeNode* type) {
        return type->compareType(this->m_type);
    }


    bool ASTVarDefNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        bool cr = m_type->checkSyntax(analyzer);
        if (!cr) return cr;

        if (analyzer->m_now_func_state == nullptr)
        {
            ASTTypeNode* var = analyzer->m_table->getGlobalVariable(m_name);
            if (var != nullptr) {
                analyzer->m_pc->addError(S("Variable '", m_name,"' has been declear before this"), analyzer);
                return false;
            }
            analyzer->m_table->addGlobalVariable(m_name, m_type);

        }
        else {
            auto idx = analyzer->m_now_func_state->m_local_variables.find(m_name);
            if (idx.isValid()) {
                analyzer->m_pc->addError("variable has been declear before this", analyzer);
                return false;
            }
            analyzer->m_now_func_state->m_local_variables.insert(m_name, m_type);
        }

        return true;
    }

} // namespace cal