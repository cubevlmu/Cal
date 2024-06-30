#include "ASTIdNode.hpp"

#include <json/json.h>
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"

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

            auto gidx = analyzer->m_global_variables.find(m_name);
            if (gidx.isValid()) return true;
            
            analyzer->m_pc->addError("variable not declear in any scope", analyzer);
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