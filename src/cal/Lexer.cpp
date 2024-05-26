#include "Lexer.hpp"

#include "base/Logger.hpp"

namespace cal {

#define LEX_TK_ADD(TK_TYPE, TK_ITEM) \
        m_tokens.push_back(Token {.tk_type = TK_TYPE, .tk_item = TK_ITEM })


    const char* tk_type_name[] = {
        "TK_EOF", "TK_SEMICOLON", "TK_EOL",
        "TK_IS_EQUAL",
        "TK_EQUAL", "TK_PLUS", "TK_MINUS", "TK_DIVID", "TK_MULTIPLE", "TK_LEFT_PAREN", "TK_RIGHT_PAREN",
        "TK_NUMBER", "TK_TEXT",
        "TK_VARIABLE",
        "TK_FUNC_CALL", "TK_FUNC_CALL_ARG", "TK_FUNC_DEF",
        "TK_UNKNOWN"
    };


    const Lexer::Token Lexer::UnknownToken = { Lexer::Token::TK_UNKNOWN, "" };


    Lexer::Lexer(const std::string& source)
        : m_src(source), m_pos(0)
    {
        if (m_src.length() == 0) {
            LogError("[Lex] Empty source");
            ASSERT(false);
        }
    }


    void Lexer::analyze() {
        LogDebug("[Lex] Source code lenght : ", m_src.length());

        while (m_pos < m_src.length()) {
            goToNextNonSpace();

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
                parseVariableOrFuncCall();
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
            else if (m_src[m_pos] == ';') {
                LEX_TK_ADD(Token::TK_SEMICOLON, ";");
                m_pos++;
            }
            else if (m_src[m_pos] == '\n') {
                LEX_TK_ADD(Token::TK_EOL, ";");
                m_pos++;
            }
            else {
                // tokens.push_back({Token::Type::INVALID, "invalid"});
                m_pos++;
            }

            // if (m_src[m_pos] == ';') {
            //     addToList(l, new Token{ Token::TK_SEMICOLON, ";" });
            // }

            //m_pos++;
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
        return m_src.substr(m_pos, token.length()) == token;
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
            parseFunctionCall();
        }
        else {
            std::string value = m_src.substr(start, m_pos - start);
            LEX_TK_ADD(Token::TK_VARIABLE, value);
        }
    }


    void Lexer::parseFunctionCall() {
        if (m_src[m_pos] != '(') return;
        m_pos++;

        while (m_src[m_pos] != ')') {
            size_t start = m_pos;
            do {
                m_pos++;
            } while (m_pos < m_src.length() && m_src[m_pos] != ',' && !std::isspace(m_src[m_pos]) && m_src[m_pos] != ')');
            std::string value = m_src.substr(start, m_pos - start);
            LEX_TK_ADD(Token::TK_FUNC_CALL_ARG, value);

            while (m_pos < m_src.length() && (std::isspace(m_src[m_pos]) || m_src[m_pos] == ','))
                m_pos++;
        }

        m_pos++; // avoid double semicolon :)
    }

} // namespace cal