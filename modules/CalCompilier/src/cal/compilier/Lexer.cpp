#include "Lexer.hpp"

#include "base/Logger.hpp"
#include <cctype>

namespace cal {

#define LEX_TK_ADD(TK_TYPE, TK_ITEM) \
        m_tokens.push(Token {.tk_type = TK_TYPE, .tk_item = TK_ITEM })


    const char* tk_type_name[] = {
        "TK_EOF", "TK_SEMICOLON", "TK_EOL",
        "TK_VAR", "TK_VAL", "TK_TYPE",
        "TK_IS_EQUAL",
        "TK_LEFT_BRACKET", "TK_RIGHT_BRACKET", "TK_LEFT_BRACES", "TK_RIGHT_BRACES",
        "TK_EQUAL", "TK_PLUS", "TK_MINUS", "TK_DIVID", "TK_MULTIPLE",
        "TK_LEFT_PAREN", "TK_RIGHT_PAREN",
        "TK_NUMBER", "TK_TEXT",
        "TK_IDENTIFIER", "TK_STRUCT", "TK_CLASS", "TK_ENUM", "TK_INTERFACE",
        "TK_DECLEAR_CONST", "TK_DECLEAR_PRIVATE", "TK_DECLEAR_PUBLIC", "TK_DECLEAR_PROTECTED", "TK_DECLEAR_INTERNAL", "TK_DECLEAR_EXPORT",
        "TK_FUNC_CALL", "TK_FUNC_ARG", "TK_FUNC_DEF", "TK_RETURN", "TK_FUNC_NAME", "TK_FUNC_RETURN",
        "TK_UNKNOWN","TK_COMMA"
    };


    const Lexer::Token Lexer::UnknownToken = { Lexer::Token::TK_UNKNOWN, "" };


    Lexer::Lexer(const std::string& source, IAllocator& alloc)
        : m_src(source), m_pos(0), m_alloc(alloc), m_tokens(alloc)
    {
        if (m_src.length() == 0) {
            LogError("[Lex] Empty source");
            ASSERT(false);
        }
    }


    void Lexer::analyze() {
        LogDebug("[Lex] Source code lenght : ", m_src.length());
        // int times = 0;

        while (m_pos < m_src.length()) {
            // times++;
            goToNextNonSpace();

            if (m_commentLineLock)
            {
            checkEOL:
                if (m_src[m_pos] == '\n') {
                    //  LEX_TK_ADD(Token::TK_EOL, ";");
                    m_pos++;
                    m_commentLineLock = false;
                }
                else if (m_src[m_pos] == '*' && m_src[m_pos + 1] == '/') {
                    m_pos += 2;
                    m_commentLineLock = false;
                    m_use_multiline_comment = false;
                }
                else if (m_use_multiline_comment) {
                    while (m_pos < m_src.length() && m_src[m_pos] != '*') {
                        if (m_src[m_pos + 1] == '/') break;
                        m_pos++;
                    }
                    goto checkEOL;
                }
                else {
                    while (m_pos < m_src.length() && m_src[m_pos] != '\n')
                        m_pos++;
                    goto checkEOL;
                }
            }

            if (m_src[m_pos] == '=') {
                if (m_src[m_pos + 1] == '=') {
                    LEX_TK_ADD(Token::TK_IS_EQUAL, "==");
                    m_pos++;
                }
                else {
                    LEX_TK_ADD(Token::TK_EQUAL, "=");
                }
                m_pos++;
            }
            else if (m_src[m_pos] == '(') {
                LEX_TK_ADD(Token::TK_LEFT_PAREN, "(");
                m_pos++;
                // m_pos++;
            }
            else if (m_src[m_pos] == ')') {
                LEX_TK_ADD(Token::TK_RIGHT_PAREN, ")");
                m_pos++;
                // m_pos++;
            }
            else if (std::isalpha(m_src[m_pos]) || m_src[m_pos] == '_') {
                if (checkTokenMatched("var ")) {
                    parseVariableDeclear(false);
                }
                else if (checkTokenMatched("val ")) {
                    parseVariableDeclear(true);
                }
                else if (checkTokenMatched("fun ")) {
                    parseFunctionDeclear();
                }
                else if (checkTokenMatched("struct ")) {
                    parseStructDeclear();
                }
                else if (checkTokenMatched("return ")) {
                    m_pos += 7;
                    LEX_TK_ADD(Token::TK_RETURN, "return");
                }
                else {
                    parseVariableOrFuncCall();
                }
            }
            else if (std::isdigit(m_src[m_pos]) || m_src[m_pos] == '-') {
                parseNumber();
            }
            else if (std::isalpha(m_src[m_pos]) || m_src[m_pos] == '\"') {
                parseText();
            }
            else if (m_src[m_pos] == '+') {
                LEX_TK_ADD(Token::TK_PLUS, "+");
                m_pos++;
            }
            else if (m_src[m_pos] == '-') {
                LEX_TK_ADD(Token::TK_MINUS, "-");
                m_pos++;
            }
            else if (m_src[m_pos] == '*') {
                LEX_TK_ADD(Token::TK_MULTIPLE, "*");
                m_pos++;
            }
            else if (m_src[m_pos] == '/' && m_src[m_pos + 1] == '/') {
                m_commentLineLock = true;
                m_pos += 2;
            }
            else if (m_src[m_pos] == '/' && m_src[m_pos + 1] == '*') {
                m_use_multiline_comment = true;
                m_commentLineLock = true;
                m_pos += 2;
            }
            else if (m_src[m_pos] == '[') {
                m_pos++;
                LEX_TK_ADD(Token::TK_LEFT_BRACKET, "[");
            }
            else if (m_src[m_pos] == '{') {
                m_pos++;
                LEX_TK_ADD(Token::TK_LEFT_BRACES, "{");
            }
            else if (m_src[m_pos] == '}') {
                m_pos++;
                LEX_TK_ADD(Token::TK_RIGHT_BRACES, "}");
            }
            else if (m_src[m_pos] == ']') {
                m_pos++;
                LEX_TK_ADD(Token::TK_RIGHT_BRACKET, "]");
            }
            else if (m_src[m_pos] == ';') {
                LEX_TK_ADD(Token::TK_SEMICOLON, ";");
                m_pos++;
            }
            else if (m_src[m_pos] == ',') {
                LEX_TK_ADD(Token::TokenType::TK_COMMA, ",");
                m_pos++;
            }
            else if (m_src[m_pos] == '\n') {
                // LEX_TK_ADD(Token::TK_EOL, ";");
                m_pos++;
                m_commentLineLock = false;
            }
            else {
                m_pos++;
            }
        }
    }


    void Lexer::debugPrint() {
        LogDebug("[LexResult] Tokens lexed : ", m_tokens.size());
        for (auto& tk : m_tokens) {
            LogDebug("\t[", tk_type_name[tk.tk_type], "] ", tk.tk_item);
        }
    }


    void Lexer::goToNextNonSpace() {
        while (m_pos < m_src.length() && std::isspace(m_src[m_pos]))
            m_pos++;
    }


    bool Lexer::checkTokenMatched(const std::string& token) {
        size_t start = m_pos;
        bool result = false;

        for (int i = 0; i < token.length(); i++) {
            result = m_src[start + i] == token[i];
            if (!result) break;
        }

        return result;
    }


    void Lexer::parseNumber() {
        size_t start = m_pos;
        bool isHex = false;

        if (m_src[m_pos] == '-') m_pos++;

        if (m_src[m_pos] == '0' && (m_src[m_pos + 1] == 'x' || m_src[m_pos + 1] == 'X')) {
            isHex = true;
            m_pos += 2; // Move past '0x' or '0X'
        }

        while (m_pos < m_src.length() && (std::isdigit(m_src[m_pos]) || (isHex && std::isxdigit(m_src[m_pos])) || m_src[m_pos] == '.'))
            m_pos++;

        std::string value = m_src.substr(start, m_pos - start);
        LEX_TK_ADD(Token::TK_NUMBER, value);
    }


    void Lexer::parseVariableDeclear(bool isConst) {
        m_pos += 4;
        LEX_TK_ADD(isConst ? Token::TK_VAL : Token::TK_VAR, isConst ? "val" : "var");
        goToNextNonSpace();

        size_t start = m_pos;
        while (m_pos < m_src.length() && (std::isalnum(m_src[m_pos]) || m_src[m_pos] == '_'))
            m_pos++;
        std::string value = m_src.substr(start, m_pos - start);
        LEX_TK_ADD(Token::TK_IDENTIFIER, value);

        goToNextNonSpace();
        if (m_src[m_pos] == ':') {
            m_pos++;
            goToNextNonSpace();
            parseDataType();
        }

        goToNextNonSpace();
        if (m_src[m_pos] == '=')
            m_pos++;
    }


    void Lexer::parseDataType() {
        //goToNextNonSpace();

        size_t start = m_pos;
        while (m_pos < m_src.length() && (std::isalnum(m_src[m_pos]) || m_src[m_pos] == '_'))
            m_pos++;
        std::string value = m_src.substr(start, m_pos - start);
        if (!std::isalpha(value[0])) {
            m_pos = start;
            return;
        }
        LEX_TK_ADD(Token::TK_TYPE, value);
    }


    void Lexer::parseText() {
        size_t start = m_pos + 1;
        size_t len = 0;
        while (true) {
            len++;
            m_pos++;
            if (m_src[m_pos] == '\'')
                break;
        }
        std::string value = m_src.substr(start, len - 1);
        m_pos++; // Move 'pos' to skip the closing quotation mark
        LEX_TK_ADD(Token::TK_TEXT, value);
    }


    void Lexer::parseVariableOrFuncCall() {
        size_t start = m_pos;
        while (m_pos < m_src.length() && (std::isalnum(m_src[m_pos]) || m_src[m_pos] == '_'))
            m_pos++;
        if (m_src[m_pos] == '(') {
            std::string value = m_src.substr(start, m_pos - start);
            LEX_TK_ADD(Token::TK_FUNC_CALL, value);
        }
        else {
            std::string value = m_src.substr(start, m_pos - start);
            LEX_TK_ADD(Token::TK_IDENTIFIER, value);
        }
    }


    void Lexer::parseFunctionCall() {
        // if (m_src[m_pos] != '(') return;
        // m_pos++;

        // while (m_src[m_pos] != ')') {
        //     size_t start = m_pos;
        //     do {
        //         m_pos++;
        //     } while (m_pos < m_src.length() && m_src[m_pos] != ',' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != ')');
        //     std::string value = m_src.substr(start, m_pos - start);

        //     LEX_TK_ADD(Token::TK_FUNC_CALL_ARG, value);

        //     while (m_pos < m_src.length() && (std::isspace(m_src[m_pos]) || m_src[m_pos] == ','))
        //         m_pos++;
        // }

        // m_pos++; // avoid double semicolon :)
    }


    void Lexer::parseFunctionDeclear() {
        m_pos += 4;
        LEX_TK_ADD(Token::TK_FUNC_DEF, "fun");

        goToNextNonSpace();
        if (std::isalpha(m_src[m_pos])) {
            size_t start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != '(' && !std::isspace(m_src[m_pos]));
            std::string value = m_src.substr(start, m_pos - start);

            LEX_TK_ADD(Token::TK_FUNC_NAME, value);
        }
        else {
            // anonymous function support
            // return;
        }

        goToNextNonSpace();
        if (m_src[m_pos] != '(')
            return;
        else
            m_pos++;

        size_t backup_point = m_pos;
        while (m_src[m_pos] != ')') {

            size_t start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != ',' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != ')');
            std::string value = m_src.substr(start, m_pos - start);

            LEX_TK_ADD(Token::TK_FUNC_ARG, value);

            goToNextNonSpace();
            if (m_src[m_pos] != ':') {
                m_pos = backup_point;
                return;
            }

            m_pos++;

            start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != ',' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != ')');
            value = m_src.substr(start, m_pos - start);

            LEX_TK_ADD(Token::TK_TYPE, value);

            while (m_pos < m_src.length() && (std::isspace(m_src[m_pos]) || m_src[m_pos] == ','))
                m_pos++;
        }

        m_pos++;
        goToNextNonSpace();

        if (m_src[m_pos] == ':') {
            m_pos++;

            size_t start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != '{' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != ';');
            std::string value = m_src.substr(start, m_pos - start);
            LEX_TK_ADD(Token::TK_FUNC_RETURN, value);
        }
        else {
            LEX_TK_ADD(Token::TK_FUNC_RETURN, "void");
            //m_pos++;
        }
        //if (m_src[m_pos] == '{') 
    }


    void Lexer::parseStructDeclear() {
        m_pos += 7;

        goToNextNonSpace();
        if (std::isalpha(m_src[m_pos])) {
            size_t start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != '(' && !std::isspace(m_src[m_pos]));
            std::string value = m_src.substr(start, m_pos - start);

            LEX_TK_ADD(Token::TK_STRUCT, value);
        }
        else {
            return;
        }

        goToNextNonSpace();
        if (m_src[m_pos] != '{')
            return;
        else
            m_pos++;

        size_t backup_point = m_pos;
        while (true) {
            goToNextNonSpace();
            if (m_src[m_pos] == '}') break;

            size_t start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != ',' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != '}');
            std::string value = m_src.substr(start, m_pos - start);

            LEX_TK_ADD(Token::TK_IDENTIFIER, value);

            goToNextNonSpace();

            if (m_src[m_pos] != ':') {
                m_pos = backup_point;
                return;
            }

            m_pos++;

            do {
                goToNextNonSpace();

                if (checkTokenMatched("const ")) {
                    LEX_TK_ADD(Token::TK_DECLEAR_CONST, "const");
                    m_pos += 6;
                }
                else if (checkTokenMatched("private ")) {
                    LEX_TK_ADD(Token::TK_DECLEAR_PRIVATE, "private");
                    m_pos += 8;
                }
                else if (checkTokenMatched("internal ")) {
                    LEX_TK_ADD(Token::TK_DECLEAR_PRIVATE, "internal");
                    m_pos += 9;
                }
                else if (checkTokenMatched("export ")) {
                    LEX_TK_ADD(Token::TK_DECLEAR_EXPORT, "export");
                    m_pos += 7;
                }
                else {
                    start = m_pos;
                    do {
                        m_pos++;
                    } while (m_pos < m_src.length() && m_src[m_pos] != ',' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != '}');
                    value = m_src.substr(start, m_pos - start);

                    LEX_TK_ADD(Token::TK_TYPE, value);
                }
            } while (m_pos < m_src.length() && m_src[m_pos] != ',' && m_src[m_pos] != '}');

            m_pos++;
        }
        m_pos++;

        goToNextNonSpace();
    }

} // namespace cal