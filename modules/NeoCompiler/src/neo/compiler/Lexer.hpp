// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include "Tokens.hpp"

#include <map>
#include <string>
#include <vector>

namespace neo {

    constexpr char CHAR_EOF = 0;
    constexpr char CHAR_CR = '\r';
    constexpr char CHAR_LF = '\n';
    constexpr char CHAR_SPACE = ' ';
    constexpr char CHAR_TAB = '\t';

#if NE_WINDOWS
#define IS_NEW_LINE(C) (C == '\n' || C == '\r')
#else
#define IS_NEW_LINE(C) (C == '\n')
#endif

    NE_FORCE_INLINE bool isDigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    NE_FORCE_INLINE bool isHexDigit(char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }
    
    NE_FORCE_INLINE bool isLetter(char ch) {
        return ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    NE_FORCE_INLINE i64 toNumber(char digit) {
        if (!isDigit(digit)) {
            return 9 + (digit & 15);
        }
        return digit & 15;
    }


    class NLexer
    {
    public:
        NLexer(NSourceFile* file);
        ~NLexer();

    public:
        bool lex();
        void debugPrint(class NDebugOutput& output);

    public:
        NToken& previousToken();
        NToken& peekPrevious();
        NToken& nextToken();
        NToken& peekNext();
        NToken& current();
        bool expectToken(TokenType);

    private:
        NE_FORCE_INLINE void skipSpace() {
            do {
                char c = m_src[m_lex_idx];
                if (!isspace(c)) {
                    break;
                }

                if (IS_NEW_LINE(c)) {
                    m_lex_line++;
                    m_lex_cursor = 0;
                }

                m_lex_idx++;
                m_lex_cursor++;
            } while (m_lex_idx < m_lex_max);
        }
        NE_FORCE_INLINE std::string subString(u32 idx, u32 len) {
            return idx < m_lex_max && idx + len < m_lex_max ?
                std::string{ m_src.data() + sizeof(char) * idx, len } : "";
        }
        bool checkMatch(const std::string_view& token) {
            if (m_lex_idx + token.length() < m_lex_max)
                return false;
            std::string_view origin { m_src.data() + sizeof(char) * m_lex_idx, token.length() };
            return origin == token;
        }

        NE_FORCE_INLINE char getChar(u32 idx) const {
            return idx > m_lex_max ? ' ' : m_src[idx];
        }

        NE_FORCE_INLINE void move(u32 idx) {
            m_lex_idx += idx;
            m_lex_cursor += idx;
        }

        NE_FORCE_INLINE void move() {
            m_lex_idx += 1;
            m_lex_cursor += 1;
        }

        NE_FORCE_INLINE void waitToLineEnd() {
            while (IS_NEW_LINE(m_src[m_lex_idx])) {
                move();
            }
        }

        NE_FORCE_INLINE void pushToken(TokenType type, const std::string& str) {
            m_tokens.push_back(NToken {
                .type = type,
                .value = str,
                .line = m_lex_line,
                .cursor = m_lex_cursor,
            });
        }

        bool lexText();
        bool lexNumber();
        bool lexIdentifier();
        bool lexComment(bool doubleSlash);

        std::string getLine();

    private:
        std::string_view m_src;
        std::vector<NToken> m_tokens;
        NSourceFile* m_source;

        psize m_tk_idx = 0;
        psize m_lex_line = 0;
        psize m_lex_idx = 0;
        psize m_lex_max = 0;
        psize m_lex_cursor = 0;

        const static std::map<char, TokenType> s_token_types;
    };
}