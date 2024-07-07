#pragma once

#include "ASTNodeBase.hpp"

namespace cal {

    class ASTAssignmentNode : public ASTNodeBase 
    {
    public:
        ASTAssignmentNode(IAllocator& alloc);
        virtual ~ASTAssignmentNode() override = default;

        virtual Json::Value buildOutput() const override;
        virtual std::string toString() const override; 
        virtual ASTNodeTypes getType() const override
        { return ASTNodeTypes::AST_ASSIGNMENT; }

        void set(const std::string& name, ASTNodeBase* value);

        ASTNodeBase* getValue() const { return m_value; }
        std::string getVarName() const { return m_var_name; }
        
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string m_var_name;
        ASTNodeBase* m_value;
    };

}