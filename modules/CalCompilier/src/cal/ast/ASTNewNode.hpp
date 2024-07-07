#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/ast/ASTTypeNode.hpp"

namespace cal {

    class ASTNewNode : public ASTNodeBase
    {
    public:
        ASTNewNode(IAllocator& alloc);
        ~ASTNewNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override
        { return ASTNodeBase::ASTNodeTypes::AST_NEW; }
        virtual std::string toString() const override; 
        virtual ASTTypeNode* getReturnType() const override;

        void set(const std::string& name);
        std::string getName() const { return m_name; }

        void setInitialValue(const std::string& member_name, ASTNodeBase* value);
        void removeInitialValueSet(const std::string& member_name);
        void clearInitialValueSet();

    private:
        std::string m_name;
        ASTTypeNode* m_insType = nullptr;
        HashMap<std::string, ASTNodeBase*> m_initial_values;
    };
}