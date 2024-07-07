#include "ASTIdNode.hpp"

#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

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


    std::string ASTIdNode::toString() const
    {
        return StringBuilder {
            ASTNodeBase::toString(),
            " [IdType:", TypeStr[(i32)m_type], "]",
            " [IdName:", m_name, "]"
        };
    }


    void ASTIdNode::set(const std::string& name, Type type) {
        m_name = name;
        m_type = type;
    }

    
    bool ASTIdNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        if (m_type == ASTIdNode::Type::FuncName) {
            //TODO
        } else if (m_type == ASTIdNode::Type::Variable) {
            if (!analyzer->m_now_func_state) {
                auto idx = analyzer->m_now_func_state->m_local_variables.find(m_name);
                if (idx.isValid()) return true;
            }

            ASTTypeNode* tp = analyzer->m_table->getGlobalVariable(m_name);
            if (tp != nullptr) return true;
            
            analyzer->m_pc->addError(S("Variable '", m_name,"' not declear in any known scope"), analyzer);
            return false;
        } else if (m_type == ASTIdNode::Type::ClassName) {
            // TODO
        } else if (m_type == ASTIdNode::Type::Define) {
            // TODO
        } else {
            return false;
        }

        return true;
    }

} // namespace cal