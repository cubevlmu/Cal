#pragma once

#include "cal/Lexer.hpp"
#include "cal/ast/ASTNodes.hpp"
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
        Parser(Lexer& lexer);

        void parse();
        void debugPrint();

    private:
        Lexer::Token peek();
        Lexer::Token advance();
        bool match(Lexer::Token::TokenType type);

        ASTBase* parseStatements();
        ASTBase* parseVariableDeclear(bool isConst);
        ASTBase* parseStatement();
        ASTBase* parseAssignment();
        ASTBase* parseFunctionCall();
        ASTBase* parseExpression();
        ASTBase* parseTerm();
        ASTBase* parseFactor();
        ASTBase* parsePrimary();
        ASTBase* parseFunctionDeclear();
        ASTBase* parseBlock();
        ASTBase* parseReturn();

    private:
        Lexer& m_lex;

        std::vector<Lexer::Token> m_tokens;
        size_t m_current;

        ASTBase* m_root;
    };
}