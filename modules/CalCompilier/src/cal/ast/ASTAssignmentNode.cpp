#include "ASTAssignmentNode.hpp"

#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

namespace cal {

    ASTAssignmentNode::ASTAssignmentNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {
    }


    Json::Value ASTAssignmentNode::buildOutput() const
    {
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["assignName"] = m_var_name;
        root["assignsVal"] = m_value->buildOutput();

        return root;
    }


    std::string ASTAssignmentNode::toString() const
    {
        return StringBuilder {
            ASTNodeBase::toString(),
            " [Name:", m_var_name, "]",
            " [Value:", m_value->toString(), "]"  
        };
    }


    void ASTAssignmentNode::set(const std::string& name, ASTNodeBase* value)
    {
        m_var_name = name;
        if(value == nullptr) {
            throw std::runtime_error("assigning value but value node is missing");
        }

        m_value = value;
    }


    bool ASTAssignmentNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        bool cr = true;
        cr = m_value->checkSyntax(analyzer);
        if(!cr) return cr;

        ASTTypeNode* real_type;
        if (analyzer->m_now_func_state != nullptr) {
            auto idx = analyzer->m_now_func_state->m_local_variables.find(m_var_name);
            if (idx.isValid()) 
                real_type = idx.value();
        } else {
            ASTTypeNode* type = analyzer->m_table->getGlobalVariable(m_var_name);
            if (type != nullptr)
                real_type = type;
        }
        if(real_type == nullptr) {
            analyzer->m_pc->addError("failed to found this variable", analyzer);
            return false;
        }

        if (!m_value->getReturnType()->compareType(real_type)) {
            analyzer->m_pc->addError("value type is not fit to the declear type", analyzer);
            return false;
        }

        return true;
    }

}