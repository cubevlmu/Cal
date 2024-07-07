#pragma once

#include "cal/ast/ASTNodeBase.hpp"

namespace cal {

    class ASTFuncReturnNode : public ASTNodeBase
    {
    public:
        enum class ReturnType {
            NormalReturn, ExceptionReturn, ErrorReturn
        };
    public:
        ASTFuncReturnNode(IAllocator& alloc);
        virtual ~ASTFuncReturnNode() override;
        
        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override
        { return ASTNodeTypes::AST_FUNC_RET; }
        virtual std::string toString() const override; 

        void set(ASTNodeBase* m_val, ReturnType type = ReturnType::NormalReturn);
        ASTNodeBase* get() const { return m_val; }
        ReturnType getRetType() const { return m_type; }
        
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        ASTNodeBase* m_val;
        ReturnType m_type;
    };
}