#pragma once

#include "cal/ast/ASTNodeBase.hpp"

namespace cal {
    
    class ASTIdNode : public ASTNodeBase
    {
    public:
        enum class Type {
            None, Variable, FuncName, ClassName, Define
        };

    public:
        ASTIdNode(IAllocator& alloc);
        virtual ~ASTIdNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override
        { return ASTNodeTypes::AST_IDENTIFIER; }

        void set(const std::string& name, Type type);
        Type getIdType() const { return m_type; }
        std::string getName() const { return m_name; }

        //TODO
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string m_name;
        Type m_type;
    };
}