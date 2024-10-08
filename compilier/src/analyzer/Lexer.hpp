#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/types/Array.hpp"
#include "utils/CPrintable.hpp"
#include "utils/StringBuilder.hpp"

#include <string>
#include <vector>

namespace cal {

    class Parser;

    class Lexer : public CPrintable
    {
        friend class Parser; 
    public:
        Lexer(const std::string& source, IAllocator& alloc);
        ~Lexer() = default;

        void analyze();
        virtual void debugPrint() const override;
        virtual std::string buildOutput() const override;

    private:
        void goToNextNonSpace();
        bool checkTokenMatched(const std::string& token);
        
        void parseNumber();
        void parseVariableDeclear(bool isConst);
        void parseDataType();
        void parseText();
        void parseVariableOrFuncCall();
        void parseFunctionCall();
        void parseFunctionDeclear();
        void parseStructDeclear();
        void parseImport();
        void parseExport();
        void parseCreateInstance();

    private:
        struct Token {
            enum TokenType {
                TK_EOF, TK_SEMICOLON, TK_EOL,
                TK_VAR, TK_VAL, TK_TYPE, TK_NEW,
                TK_IS_EQUAL, 
                TK_LEFT_BRACKET, TK_RIGHT_BRACKET, TK_LEFT_BRACES, TK_RIGHT_BRACES,
                TK_EQUAL, TK_PLUS, TK_MINUS, TK_DIVID, TK_MULTIPLE, 
                TK_LEFT_PAREN, TK_RIGHT_PAREN,
                TK_NUMBER, TK_TEXT, 
                TK_IDENTIFIER, TK_STRUCT, TK_CLASS, TK_ENUM, TK_INTERFACE, TK_MODULE, TK_IMPORT, TK_EXPORT, TK_EXTERN,
                TK_DECLEAR_CONST, TK_DECLEAR_PRIVATE, TK_DECLEAR_PUBLIC, TK_DECLEAR_PROTECTED, TK_DECLEAR_INTERNAL, TK_DECLEAR_EXPORT,
                TK_FUNC_CALL, TK_FUNC_ARG, TK_FUNC_DEF, TK_RETURN, TK_FUNC_NAME, TK_FUNC_RETURN,
                TK_MODULE_NAME, TK_EXPORT_ARG,
                TK_UNKNOWN, TK_COMMA, TK_DOT
            };
            const TokenType tk_type;
            const std::string tk_item;
        };

        static const Token UnknownToken;

        Array<Token> m_tokens;
        const std::string m_src;
        size_t m_pos;
        bool m_commentLineLock = false;
        bool m_use_multiline_comment = false;
        IAllocator& m_alloc;
        StringBuilder m_builder;
    };
}