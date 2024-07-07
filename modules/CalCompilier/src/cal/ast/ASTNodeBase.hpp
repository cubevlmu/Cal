#pragma once

#include <string>
#include "base/allocator/IAllocator.hpp"

namespace Json {
    class Value;
}

namespace cal {

    class ASTTypeNode;
    class SyntaxAnalyzer;
    class GrammarAnalyzer;
    class CodeGenerator;
    class CodeOptimizer;

    class ASTNodeBase
    {
    public:
        /// @brief Node types for AST nodes
        enum class ASTNodeTypes {
            AST_OP, AST_NUMBER, AST_TEXT, AST_IDENTIFIER,
            AST_BLOCK, AST_TYPE, AST_FUNC_DEF, AST_FUNC_ARG_DEF,
            AST_VAR, AST_ASSIGNMENT, AST_FUNC_CALL, AST_FUNC_RET,
            AST_STRUCT, AST_MODULE, AST_MODULE_IMP, AST_NEW
        };

        /// @brief AST Type's text description
        static const char* ASTNodeTypesString[];
        static std::string getNodeString(ASTNodeTypes node) {
            return ASTNodeTypesString[(i32)node];
        }

    public:
        explicit ASTNodeBase(IAllocator& alloc);
        virtual ~ASTNodeBase();

        /// @brief Build the detail information about this node
        /// @return JsonCpp's Value object
        virtual Json::Value buildOutput() const = 0;
        /// @brief Get current node's node type
        /// @return Node's type
        virtual ASTNodeTypes getType() const = 0;
        /// @brief Current node's return value type
        /// @brief For assginment or function calling etc.
        /// @return ASTTypeNode pointer
        virtual ASTTypeNode* getReturnType() const;
        /// @brief Output detail about this node
        /// @return If not override by subclass, it would output the node's type string
        virtual std::string toString() const;
        /// @brief Return current node's node type name
        /// @return type name
        std::string getNodeName() const { return ASTNodeTypesString[(i32)getType()]; }

        /// @brief Collecting symbol for compiling
        /// @param analyzer analyzer pass
        /// @return state of checking
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const { return false; }
        /// @brief Check grammar is valid or not
        /// @param checker checker pass
        /// @return state of checking
        virtual bool grammarCheck(GrammarAnalyzer* checker) const { return false; }
        virtual bool optimizeSelf(CodeOptimizer* optimizer) const { return false; }
        virtual bool codeGen(CodeGenerator* generator) const { return false; }

        ASTNodeBase* getParent() const {
            return m_parent;
        }
        void setParent(ASTNodeBase* parent) {
            m_parent = parent;
        }

    protected:
        ASTNodeBase* m_parent = nullptr;
        IAllocator& m_allocator;
    };
}