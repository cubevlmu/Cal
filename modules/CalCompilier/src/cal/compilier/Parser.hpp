#pragma once

#include "Lexer.hpp"
#include "base/allocator/TagAllocator.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include <cstddef>

namespace cal {

    class TypeChecker;
    class CodeGenerator;
    class LLVMCodeGenerator;

    class Parser 
    {
        friend class TypeChecker;
        friend class CodeGenerator;
        friend class LLVMCodeGenerator;
        
    public:
        Parser(Lexer& lexer, IAllocator& alloc);

        void parse();
        void debugPrint();
        ASTNodeBase* getAST() {
            return m_root;
        }

    private:
        Lexer::Token peek();
        Lexer::Token advance();
        bool match(Lexer::Token::TokenType type);

        ASTNodeBase* parseStatements();
        ASTNodeBase* parseVariableDeclear(bool isConst);
        ASTNodeBase* parseStatement();
        ASTNodeBase* parseAssignment();
        ASTNodeBase* parseFunctionCall();
        ASTNodeBase* parseExpression();
        ASTNodeBase* parseTerm();
        ASTNodeBase* parseFactor();
        ASTNodeBase* parsePrimary();
        ASTNodeBase* parseFunctionDeclear();
        ASTNodeBase* parseBlock();
        ASTNodeBase* parseReturn();
        ASTNodeBase* parseStructDeclear();
        ASTNodeBase* parseCreateInstance();

    private:
        Lexer& m_lex;
        IAllocator& m_alloc;
        TagAllocator m_allocator;

        Array<Lexer::Token> m_tokens;
        size_t m_current;

        ASTNodeBase* m_root;
    };
}