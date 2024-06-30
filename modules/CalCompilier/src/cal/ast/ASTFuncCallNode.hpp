#pragma once

#include "base/types/Array.hpp"
#include "cal/ast/ASTNodeBase.hpp"

namespace cal {

    class ASTFuncCallNode : public ASTNodeBase
    {
    public:
        ASTFuncCallNode(IAllocator& alloc);
        virtual ~ASTFuncCallNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override
        {
            return ASTNodeTypes::AST_FUNC_CALL;
        }
        virtual ASTTypeNode* getReturnType() const override {
            return m_ret_type;
        }

        void setName(const std::string& name);
        std::string getName() const { return m_name; }

        void addParam(ASTNodeBase* node);
        void delParam(ASTNodeBase* node);
        void clearParams();

        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string m_name;
        Array<ASTNodeBase*> m_args;
        mutable ASTTypeNode* m_ret_type;
    };
}