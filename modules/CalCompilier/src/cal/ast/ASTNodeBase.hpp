#pragma once

#include <string>
#include "base/allocator/IAllocator.hpp"

namespace Json {
    class Value;
}

namespace cal {

    class ASTTypeNode;
    class SyntaxAnalyzer;
    class GrammarChecker;
    class CodeGenerator;
    class CodeOptimizer;

    class ASTNodeBase
    {
    public:
        enum class ASTNodeTypes {
            AST_OP, AST_NUMBER, AST_TEXT, AST_IDENTIFIER,
            AST_BLOCK, AST_TYPE, AST_FUNC_DEF, AST_FUNC_ARG_DEF,
            AST_VAR, AST_ASSIGNMENT, AST_FUNC_CALL, AST_FUNC_RET
        };

        static const char* ASTNodeTypesString[];

    public:
        explicit ASTNodeBase(IAllocator& alloc);
        virtual ~ASTNodeBase();

        virtual Json::Value buildOutput() const = 0;
        virtual ASTNodeTypes getType() const = 0;
        virtual ASTTypeNode* getReturnType() const;
        virtual std::string toString() const;
        std::string getNodeName() const { return ASTNodeTypesString[(i32)getType()]; }

        /// @brief Check variable declear is vaild
        /// @param analyzer analyzer pass
        /// @return state of checking
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const { return true; }
        virtual bool grammarCheck(GrammarChecker* checker) const { return true; }
        virtual bool codeGen(CodeGenerator* generator) const { return true; }
        virtual bool optimizeSelf(CodeOptimizer* optimizer) const { return true; }

    public:
        ASTNodeBase* m_parent = nullptr;
        ASTNodeBase* m_child = nullptr;

    protected:
        IAllocator& m_allocator;
    };
}