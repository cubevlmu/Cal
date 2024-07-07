#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/ast/ASTTypeNode.hpp"
namespace cal {

    enum StructMemberAccessibility : u32 {
        DEFAULT = 0,
        INTERNAL = 1 << 0,
        CONST = 1 << 1,
        STATIC = 1 << 2
    };

    class ASTStructNode : public ASTNodeBase
    {
    public:
        struct StructMember {
            ASTTypeNode* type;
            StructMemberAccessibility accessibility;
            ASTNodeBase* defaultValue = nullptr;
        };

    public:
        ASTStructNode(IAllocator& alloc);

        virtual Json::Value buildOutput() const override;
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;
        virtual std::string toString() const override; 
        virtual ASTNodeTypes getType() const override
        {
            return ASTNodeBase::ASTNodeTypes::AST_STRUCT;
        }

        void set(const std::string& name);

        void addMember(const std::string& var_name, ASTTypeNode* type, StructMemberAccessibility ass = StructMemberAccessibility::DEFAULT);
        void removeMember(const std::string& var_name);
        void clearMember();
        bool registerTo(SyntaxAnalyzer* analyzer);

        std::string getName() const { return m_name; }
        ASTTypeNode* getMemberType(const std::string& name) const;
        StructMemberAccessibility getMemberAccessibility(const std::string& name) const;

    private:
        HashMap<std::string, StructMember> m_members;
        std::string m_name;
    };


    constexpr StructMemberAccessibility operator ~(StructMemberAccessibility a) { return StructMemberAccessibility(~(u32)a); }
    constexpr StructMemberAccessibility operator | (StructMemberAccessibility a, StructMemberAccessibility b) { return StructMemberAccessibility((u32)a | (u32)b); }
    constexpr StructMemberAccessibility operator & (StructMemberAccessibility a, StructMemberAccessibility b) { return StructMemberAccessibility((u32)a & (u32)b); }
}