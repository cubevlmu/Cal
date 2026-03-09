/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "Tokens.hpp"
#include "neo/diagnose/Diagnostic.hpp"

#include <map>

namespace neo
{
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

    NE_FORCE_INLINE bool isDigit(char ch)
    {
        return ch >= '0' && ch <= '9';
    }

    NE_FORCE_INLINE bool isHexDigit(char ch)
    {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }

    NE_FORCE_INLINE bool isLetter(char ch)
    {
        return ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    NE_FORCE_INLINE i64 toNumber(char digit)
    {
        if (!isDigit(digit))
        {
            return 9 + (digit & 15);
        }
        return digit & 15;
    }

    class NLexer
    {
        friend class NParser;

    public:
        NLexer(NSourceFile *file);
        ~NLexer();

    public:
        bool lex();
        void debugPrint(class NDebugOutput &output);
        void printDiagnostics() const;

    public:
        NToken &previousToken();
        NToken &peekPrevious();
        NToken &nextToken();
        NToken &peekNext();
        NToken &current();
        bool expectToken(TokenType);

    private:
        NE_FORCE_INLINE void skipSpace()
        {
            do
            {
                char c = m_src[m_lex_idx];
                if (!isspace(c))
                {
                    break;
                }

                if (IS_NEW_LINE(c))
                {
                    m_lex_line++;
                    m_lex_cursor = 0;
                }

                m_lex_idx++;
                m_lex_cursor++;
            } while (m_lex_idx < m_lex_max);
        }
        NE_FORCE_INLINE String subString(u32 idx, u32 len)
        {
            return idx < m_lex_max && idx + len < m_lex_max ? String{m_src.data() + sizeof(char) * idx, len} : "";
        }
        bool checkMatch(const StringView &token)
        {
            if (m_lex_idx + token.length() < m_lex_max)
                return false;
            StringView origin{m_src.data() + sizeof(char) * m_lex_idx, token.length()};
            return origin == token;
        }

        NE_FORCE_INLINE char getChar(u32 idx) const
        {
            return idx > m_lex_max ? ' ' : m_src[idx];
        }

        NE_FORCE_INLINE void move(u32 idx)
        {
            m_lex_idx += idx;
            m_lex_cursor += idx;
        }

        NE_FORCE_INLINE void move()
        {
            m_lex_idx += 1;
            m_lex_cursor += 1;
        }

        NE_FORCE_INLINE void waitToLineEnd()
        {
            while (IS_NEW_LINE(m_src[m_lex_idx]))
            {
                move();
            }
        }

        NE_FORCE_INLINE void pushToken(TokenType type, const String &str)
        {
            m_tokens.push_back(NToken{
                .type = type,
                .value = str,
                .line = m_lex_line,
                .cursor = m_lex_cursor,
            });
        }

        NE_FORCE_INLINE void pushTokenAt(TokenType type, const String &str, psize line, psize cursor)
        {
            m_tokens.push_back(NToken{
                .type = type,
                .value = str,
                .line = line,
                .cursor = cursor,
            });
        }

        bool lexText();
        bool lexNumber();
        bool lexIdentifier();
        bool lexComment(bool doubleSlash);
        bool failAtCurrent(const String &message);
        bool fail(psize line, psize cursor, const String &message);

        String getLine();

    private:
        StringView m_src;
        Vector<NToken> m_tokens;
        NSourceFile *m_source;

        psize m_tk_idx = 0;
        psize m_lex_line = 0;
        psize m_lex_idx = 0;
        psize m_lex_max = 0;
        psize m_lex_cursor = 0;

        DiagnosticCollector m_diag;

        const static std::map<char, TokenType> s_token_types;
    };
}
