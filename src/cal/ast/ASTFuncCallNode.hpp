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
        { return ASTNodeTypes::AST_FUNC_CALL; }

        void setName(const std::string& name);
        std::string getName() const { return m_name; }

        void addParam(ASTNodeBase* node);
        void delParam(ASTNodeBase* node);
        void clearParams();

    private:
        std::string m_name;
        Array<ASTNodeBase*> m_args;
    };
}