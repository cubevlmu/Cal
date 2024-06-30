#pragma once

#include "ASTNodeBase.hpp"

namespace cal {

    class ASTOPNode : public ASTNodeBase
    {
    public:
        enum class OperatorType {
            Add,        // 加法 +
            Subtract,   // 减法 -
            Multiply,   // 乘法 *
            Divide,     // 除法 /
            Modulus,    // 取模 %

            And,        // 逻辑与 &&
            Or,         // 逻辑或 ||
            Not,        // 逻辑非 !

            Equal,      // 等于 ==
            NotEqual,   // 不等于 !=
            LessThan,   // 小于 <
            GreaterThan,// 大于 >
            LessOrEqual,// 小于等于 <=
            GreaterOrEqual // 大于等于 >=
        };

    public:
        ASTOPNode(IAllocator& alloc);
        virtual ~ASTOPNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override 
        { return ASTNodeTypes::AST_OP; }
        virtual ASTTypeNode* getReturnType() const override;

        void set(const OperatorType& type, ASTNodeBase* left, ASTNodeBase* right);

        ASTNodeBase* getLeft() { return m_left; }
        ASTNodeBase* getRight() { return m_right; }
        OperatorType getOpType() const { return m_type; }

        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;
        
    private:
        ASTNodeBase *m_left, *m_right;
        OperatorType m_type;
    };
}