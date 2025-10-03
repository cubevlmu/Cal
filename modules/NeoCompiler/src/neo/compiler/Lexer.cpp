// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "neo/compiler/Lexer.hpp"

#include "neo/compiler/DebugOutput.hpp"
#include "neo/compiler/SourceFile.hpp"

#include <nbase/base/Logger.hpp>

namespace neo {

    const std::map<char, TokenType> NLexer::s_token_types = {
        {'+', TokenType::kAdd},
        {'-', TokenType::kSub},
        {'*', TokenType::kMul},
        // {'/', TokenType::kDiv},
        {'%', TokenType::kMod},

        {'<', TokenType::kLt},
        {'>', TokenType::kGt},
        {'=', TokenType::kAssign},
        {'!', TokenType::kLNot},

        {'&', TokenType::kBitAnd},
        {'|', TokenType::kBitOr},
        {'^', TokenType::kBitXor},
        {'~', TokenType::kBitNot},

        {'(', TokenType::kLParen},
        {')', TokenType::kRParen},
        {'{', TokenType::kLBraces},
        {'}', TokenType::kRBraces},
        {'[', TokenType::kLBracket},
        {']', TokenType::kRBracket},

        {'.', TokenType::kDot},
        {':', TokenType::kColon},
        {';', TokenType::kSemicolon},
        {',', TokenType::kComma},

        {'?', TokenType::kQuestion},
    };


#define LEX_TOKEN(TYPE, VALUE) NToken { .type = TYPE, .value = VALUE, .line = m_lex_line, .cursor = m_lex_cursor  }


    NLexer::NLexer(NSourceFile* file)
        : m_tokens{}
        , m_src{ file->getContent().data() }
        , m_lex_max{ (u32)m_src.length() }
        , m_source{ file }
    {
    }


    NLexer::~NLexer()
    {
        m_tokens.clear();
    }


    bool NLexer::lex()
    {
        m_tokens.clear();
        m_lex_line = 1;
        m_lex_cursor = 1;

        do {
            skipSpace();
            if (m_lex_idx == m_lex_max) {
                m_tokens.push_back(LEX_TOKEN(TokenType::kEOF, ""));
                break;
            }

            char c = m_src[m_lex_idx];
            // lexer logic
            if (s_token_types.find(c) != s_token_types.end()) {
                TokenType type = s_token_types.at(c);
                char next = m_src[m_lex_idx + 1];

                if (type == TokenType::kAssign) {
                    if (next == '=') {
                        pushToken(TokenType::kEq, "==");
                        goto end;
                    }
                }
                else if (type == TokenType::kLNot) {
                    if (next == '=') {
                        pushToken(TokenType::kNeq, "!=");
                        goto end;
                    }
                }
                else if (type == TokenType::kAdd) {
                    if (next == '=') {
                        pushToken(TokenType::kAddAssign, "+=");
                        goto end;
                    }
                    else if (next == '+') {
                        pushToken(TokenType::kInc, "++");
                        goto end;
                    }
                }
                else if (type == TokenType::kSub) {
                    if (next == '=') {
                        pushToken(TokenType::kSubAssign, "-=");
                        goto end;
                    }
                    else if (next == '-') {
                        pushToken(TokenType::kDec, "--");
                        goto end;
                    }
                }
                else if (type == TokenType::kMul) {
                    if (next == '=') {
                        pushToken(TokenType::kMulAssign, "*=");
                        goto end;
                    }
                }
                else if (type == TokenType::kColon) {
                    if (next == ':') {
                        pushToken(TokenType::kDoubleColon, "::");
                        goto end;
                    }
                }
                else if (type == TokenType::kDiv) {
                    if (next == '=') {
                        pushToken(TokenType::kDivAssign, "/=");
                        goto end;
                    }
                }
                else if (type == TokenType::kMod) {
                    if (next == '=') {
                        pushToken(TokenType::kModAssign, "%=");
                        goto end;
                    }
                }
                else if (type == TokenType::kLt) {
                    if (next == '=') {
                        pushToken(TokenType::kLe, "<=");
                        goto end;
                    }
                    else if (next == '<') {
                        char third = m_src[m_lex_idx + 2];
                        if (third == '=') {
                            pushToken(TokenType::kShlAssign, "<<=");
                            move();  // move extra once below
                            goto end;
                        }
                        else {
                            pushToken(TokenType::kShl, "<<");
                            goto end;
                        }
                    }
                }
                else if (type == TokenType::kGt) {
                    if (next == '=') {
                        pushToken(TokenType::kGe, ">=");
                        goto end;
                    }
                    else if (next == '>') {
                        char third = m_src[m_lex_idx + 2];
                        if (third == '=') {
                            pushToken(TokenType::kShrAssign, ">>=");
                            move();  // move extra once below
                            goto end;
                        }
                        else {
                            pushToken(TokenType::kShr, ">>");
                            goto end;
                        }
                    }
                }
                else if (type == TokenType::kBitAnd) {
                    if (next == '=') {
                        pushToken(TokenType::kAndAssign, "&=");
                        goto end;
                    }
                    else if (next == '&') {
                        pushToken(TokenType::kLAnd, "&&");
                        goto end;
                    }
                }
                else if (type == TokenType::kBitOr) {
                    if (next == '=') {
                        pushToken(TokenType::kOrAssign, "|=");
                        goto end;
                    }
                    else if (next == '|') {
                        pushToken(TokenType::kLOr, "||");
                        goto end;
                    }
                }
                else if (type == TokenType::kBitXor) {
                    if (next == '=') {
                        pushToken(TokenType::kXorAssign, "^=");
                        goto end;
                    }
                }

                m_tokens.push_back(LEX_TOKEN(type, { c }));
                move();
                continue;
            end:
                move(2);
                // 额外处理 <<==、>>== 的多字符情况
                // if ((type == TokenType::kLt || type == TokenType::kGt) && next == m_src[m_lex_idx]) {
                //     // 多移动一次
                //     move();
                // }
            }
            else if (c == '/') {
                char next = getChar(m_lex_idx + 1);
                if (next == '/') {
                    move(2);
                    auto e = lexComment(true);
//                    if (!e) {
//                        LogError("[Lexer] Failed to lex comment");
//                    }
                }
                else if (next == '*') {
                    move(2);
                    auto e = lexComment(false);
//                    if (!e) {
//                        LogError("[Lexer] Failed to lex comment");
//                    }
                }
                else {
                    move(1);
                    m_tokens.push_back(LEX_TOKEN(TokenType::kDiv, "/"));
                }
                continue;
            }
            else if (c == '\'') {
                if (m_lex_idx + 2 < m_lex_max) {
                    char c2 = m_src[m_lex_idx + 2];
                    if (c2 == '\'') {
                        move(2);
                        m_tokens.push_back(LEX_TOKEN(TokenType::kCharLit, std::string{ m_src[m_lex_idx + 1] }));
                        continue;
                    }
                }
            }
            else if (c == '\"') {
                auto txt = lexText();
                if (!txt) {
                    LogError("Failed to lex text");
                }
            }
            else if (c == CHAR_EOF) {
                m_tokens.push_back(LEX_TOKEN(TokenType::kEOF, ""));
                break;
            }
            else {
                if (isDigit(c)) {
                    bool e = lexNumber();
                    if (!e) {
                        LogError("scan number result -> ", c);
                        LogError("[", m_lex_line, ':', m_lex_cursor, "] '", getLine(), "'");
                    }
                }
                else if (isLetter(c)) {
                    bool e = lexIdentifier();
//                    if (!e) {
//                        LogError("[Lexer] scan identifier result -> ", c);
//                    }
                }
                else if (c == CHAR_EOF) {
                    m_tokens.push_back(LEX_TOKEN(TokenType::kEOF, ""));
                    return true;
                }
                else {
                    LogError("[Lexer] unexpected symbol near -> ", c);
                    return false;
                }
                continue;
            }
        } while (m_lex_idx < m_lex_max);

        return true;
    }


    void NLexer::debugPrint(NDebugOutput& output)
    {
        output.write("Lex result of file : ");
        output.writeLine(m_source->getPath());
        output.writeLine("     ");
        for (auto& tk : m_tokens) {
            output.write("\t");
            output.writeLine(tk.toString());
        }
    }


    NToken& NLexer::previousToken()
    {
        if (m_tk_idx > 0)
            m_tk_idx--;
        return m_tokens[m_tk_idx];
    }


    NToken& NLexer::peekPrevious()
    {
        if (m_tk_idx > 0)
            return m_tokens[m_tk_idx - 1];
        return NToken::Invalid;
    }


    NToken& NLexer::nextToken()
    {
        m_tk_idx++;
        if (m_tk_idx >= m_tokens.size())
            m_tk_idx = m_tokens.size() - 1;
        return m_tokens[m_tk_idx];
    }


    NToken& NLexer::peekNext()
    {
        if (m_tk_idx + 1 >= m_tokens.size())
            return NToken::Invalid;
        return m_tokens[m_tk_idx + 1];
    }


    NToken& NLexer::current() {
        if (m_tk_idx < 0 || m_tk_idx >= m_tokens.size()) {
            return NToken::Invalid;
        }
        return m_tokens[m_tk_idx];
    }


    bool NLexer::expectToken(TokenType tp)
    {
        NToken& tk = peekNext();
        return tk.type == tp;
    }


    bool NLexer::lexText()
    {
        u32 start = m_lex_idx + 1;
        char current;
        std::string result {};
        move();

        while (m_lex_idx < m_lex_max) {
            current = getChar(m_lex_idx);
            if (current == 0 || current == '\r' || current == '\n') {
                char prev = getChar(m_lex_idx - 1);
                if (prev != '\\') {
                    LogError("[Lexer] expect another quotation mark before end of file or newline");
                    return false;
                }
//                u32 endLine = m_lex_idx;
                auto linePart = subString(start, m_lex_idx - start);
                result.append(linePart);
                move();
                start = m_lex_idx;
                continue;
            }
            if (current == '\"') {
                auto str = subString(start, m_lex_idx - start);
                result.append(str);
                move();
                pushToken(TokenType::kStringLit, result);
                return true;
            }
            move();
        }

        pushToken(TokenType::kStringLit, result);
        return true;
    }


    bool NLexer::lexNumber()
    {
        u32 start = m_lex_idx;
//        bool isFloat;

        char first = getChar(m_lex_idx);
        if (first == '0') {
            char next = getChar(m_lex_idx + 1);
            if (next == 'X' || next == 'x') {
                move(2);
                while (m_lex_idx < m_lex_max && isHexDigit(m_src[m_lex_idx]) && IS_NEW_LINE(m_src[m_lex_idx])) {
                    move();
                }
                auto hex = subString(start, m_lex_idx - start);
                pushToken(TokenType::kHexLit, hex);
                return true;
            }
        }
        if (!isDigit(first)) {
            return false;
        }
        while (m_lex_idx < m_lex_max) {
            char current = m_src[m_lex_idx];
            if (!isDigit(current) || current == '\n' || current == '\r' || current == 0) {
                break;
            }
//TODO            else if (current == '.') {
//                if (isFloat) {
//                    return false;
//                }
//            }
            move();
        }
        auto number = subString(start, m_lex_idx - start);
        if (number.find('.') != std::string::npos) {
            pushToken(TokenType::kFloatLit, number);
        }
        else {
            pushToken(TokenType::kIntLit, number);
        }
        return true;
    }


    bool NLexer::lexIdentifier()
    {
        u32 start = m_lex_idx;

        while (m_lex_idx < m_lex_max && isLetter(m_src[m_lex_idx]) || isDigit(m_src[m_lex_idx])) {
            move();
        }

        auto idType = subString(start, m_lex_idx - start);
        auto type = NToken::checkIdentifier(idType.c_str());
        if (type != TokenType::kUnknown) {
            pushToken(type, idType);
            return true;
        }

        pushToken(TokenType::kIdentifier, idType);
        return true;
    }


    bool NLexer::lexComment(bool doubleSlash)
    {
        if (doubleSlash) {
//            u32 start = m_lex_idx;
            while (m_lex_idx < m_lex_max) {
                char ahead = getChar(m_lex_idx);
                if (ahead == CHAR_EOF || ahead == CHAR_CR || ahead == CHAR_LF) {
                    break;
                }
                move();
            }
            // pushToken(TokenType::CommentSingleLine, m_src.substr(start, m_lex_idx - start));
            return true;
        }
        /* */

//        u32 start = m_lex_idx;
        while (m_lex_idx < m_lex_max) {
            char current = getChar(m_lex_idx);
            if (current == '*') {
                char next = getChar(m_lex_idx + 1);
                if (next == '/') {
                    move(2);
                    break;
                }
            }
            if (IS_NEW_LINE(current)) {
                m_lex_line++;
                m_lex_cursor = 0;
            }
            move();
        } //TODO how about colum ? line ? fix it later
        // pushToken(TokenType::CommentMultiLine, m_src.substr(start, m_lex_idx - start));
        return true;
    }


    std::string NLexer::getLine()
    {
        u32 idx = m_lex_idx;

        while (m_lex_idx > 0) {
            m_lex_idx--;
            if (IS_NEW_LINE(m_src[m_lex_idx])) {
                break;
            }
        }
        u32 start = m_lex_idx + 1;
        m_lex_cursor = idx;

        while (m_lex_idx < m_lex_max) {
            m_lex_idx++;
            if (IS_NEW_LINE(m_src[m_lex_idx])) {
                break;
            }
        }
        auto str = subString(start, m_lex_idx - start);
        m_lex_idx = idx;
        return str;
    }

}