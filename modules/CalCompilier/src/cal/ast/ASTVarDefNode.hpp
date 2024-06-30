#pragma once

#include "ASTNodeBase.hpp"

namespace cal {

    class ASTVarDefNode : public ASTNodeBase
    {
    public:
        ASTVarDefNode(IAllocator& alloc);
        virtual ~ASTVarDefNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTTypeNode* getReturnType() const override
        { return m_type; }
        virtual ASTNodeTypes getType() const override
        { return ASTNodeTypes::AST_VAR;  }

        void set(const std::string& name, ASTTypeNode* node, ASTNodeBase* initial_value = nullptr);
        void set(const std::string& name, ASTNodeBase* initial_value);

        std::string getVarName() const { return m_name; }
        ASTTypeNode* getVarType() const { return m_type; }
        ASTNodeBase* getValue() const { return m_initial_value; }
        
        bool compareType(ASTTypeNode* type);

        void setConstant(bool cst) { m_const = cst; }
        bool getIsConstant() const { return m_const; }
        
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string  m_name;
        ASTTypeNode* m_type;
        ASTNodeBase* m_initial_value;
        bool         m_const;
    };
}