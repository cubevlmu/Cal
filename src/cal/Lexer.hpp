#pragma once

#include <string>
#include <vector>

namespace cal {

    class Parser;

    class Lexer 
    {
        friend class Parser; 
    public:
        Lexer(const std::string& source);
        ~Lexer() = default;

        void analyze();
        void debugPrint();

    private:
        void goToNextNonSpace();
        bool checkTokenMatched(const std::string& token);
        
        void parseNumber();
        void parseText();
        void parseVariableOrFuncCall();
        void parseFunctionCall();

    private:
        struct Token {
            enum TokenType {
                TK_EOF, TK_SEMICOLON, TK_EOL,
                TK_IS_EQUAL, 
                TK_EQUAL, TK_PLUS, TK_MINUS, TK_DIVID, TK_MULTIPLE, TK_LEFT_PAREN, TK_RIGHT_PAREN,
                TK_NUMBER, TK_TEXT, 
                TK_VARIABLE,
                TK_FUNC_CALL, TK_FUNC_CALL_ARG, TK_FUNC_DEF,
                TK_UNKNOWN
            };
            TokenType tk_type;
            std::string tk_item;
        };

        static const Token UnknownToken;

        std::vector<Token> m_tokens;
        std::string m_src;
        size_t m_pos;
        bool m_commentLineLock = false;
    };
}