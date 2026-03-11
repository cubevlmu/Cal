/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Parser.hpp"

#include "neo/ast/Type.hpp"
#include "neo/ast/Decl.hpp"
#include "neo/diagnose/Diagnostic.hpp"
#include "neo/compiler/Lexer.hpp"
#include "neo/compiler/Tokens.hpp"
#include "ParsedFile.hpp"
#include "neo/ast/Stmts.hpp"
#include "neo/ast/Exprs.hpp"
#include "neo/diagnose/ScopeGuard.hpp"

#include <nbase/types/String.hpp>
#include <nbase/utils/StringUtils.hpp>
#include <cctype>
#include <vector>

// HINT: all statement parser should advence at last token
//       like ';' at statement's end or ')'/']'/'}' at scope end

// TODO add end line check

#if NE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
#endif

namespace neo
{

    TokenType NParser::s_modifier[] = {
        TokenType::kInline,
        TokenType::kStatic,
        TokenType::kConst,
        TokenType::kExport,
        TokenType::kInternal,
        TokenType::kProtected,
        TokenType::kPrivate,
        TokenType::kFinal,
        TokenType::kVirtual,
        TokenType::kOverride,
        TokenType::kImpl,
    };

#define CHECK_MODIFIER(ITEM, ITEM_NAME)                            \
    if (ITEM)                                                      \
    {                                                              \
        return Result::failure("duplicated modifier " #ITEM_NAME); \
    }
#define ERRR() &m_diag, current(), m_args.file
#define CLEARUP(V)               \
    do                           \
    {                            \
        for (auto *ptr : V)      \
        {                        \
            neo::deletePtr(ptr); \
        }                        \
        V.clear();               \
    } while (false)

    NParser::NParser(NParserArgs args)
        : m_args{args}, m_lexer{args.lexer}
    {
    }

    NParser::~NParser()
    {
        m_diag.clear();
    }

    NToken &NParser::advance()
    {
        return m_lexer->nextToken();
    }
    NToken &NParser::current()
    {
        return m_lexer->current();
    }
    NToken &NParser::peek()
    {
        return m_lexer->peekNext();
    }
    NToken &NParser::peekPrevious()
    {
        return m_lexer->peekPrevious();
    }
    NToken &NParser::previous()
    {
        return m_lexer->previousToken();
    }
    bool NParser::match(TokenType type)
    {
        return current().type == type;
    }
    bool NParser::expect(TokenType type)
    {
        return m_lexer->expectToken(type);
    }
    bool NParser::check(TokenType type)
    {
        return m_lexer->current().type == type;
    }

    String NParser::tokenText(const NToken &token) const
    {
        if (!token.value.empty())
        {
            return "'" + token.value + "'";
        }
        return "'" + String(token.typeString()) + "'";
    }

    Result NParser::unexpectedToken(const String &context)
    {
        return Result::failure("unexpected token " + tokenText(current()) + " in " + context, ERRR());
    }

    Result NParser::expectedToken(const String &expected, const String &context)
    {
        return Result::failure("expected " + expected + " in " + context + ", found " + tokenText(current()), ERRR());
    }

    void NParser::synchronize(std::initializer_list<TokenType> tokens, bool consumeToken)
    {
        while (!check(TokenType::kEOF))
        {
            for (auto t : tokens)
            {
                if (check(t))
                {
                    if (consumeToken)
                    {
                        advance();
                    }
                    return;
                }
            }
            advance();
        }
    }

    void NParser::synchronizeTopLevel()
    {
        synchronize({TokenType::kImport, TokenType::kModule, TokenType::kFun, TokenType::kClass, TokenType::kStruct,
                     TokenType::kInterface, TokenType::kEnum, TokenType::kVar, TokenType::kVal, TokenType::kConst,
                     TokenType::kExport, TokenType::kLBracket, TokenType::kEOF});
    }

    void NParser::synchronizeStmt()
    {
        while (!check(TokenType::kEOF))
        {
            if (check(TokenType::kSemicolon))
            {
                advance();
                return;
            }
            if (check(TokenType::kRBraces))
            {
                return;
            }
            advance();
        }
    }

    void NParser::synchronizeExpr()
    {
        synchronize({TokenType::kComma, TokenType::kRParen, TokenType::kRBracket, TokenType::kSemicolon, TokenType::kRBraces});
    }

    bool NParser::shouldAbort() const
    {
        return m_diag.getErrorCount() >= s_maxRecoverErrors;
    }

    ASTExpr *NParser::makeErrorExpr(const NToken &start)
    {
        return setLoc(new ErrorExpr(), start);
    }

    ASTStmt *NParser::makeErrorStmt(const NToken &start)
    {
        return setLoc(new ErrorStmt(), start);
    }

    ASTDecl *NParser::makeErrorDecl(const NToken &start)
    {
        return setLoc(new ErrorDecl(), start);
    }

    bool NParser::isTypeAt(psize idx, psize *endIdx) const
    {
        const auto &tokens = m_lexer->m_tokens;
        if (idx >= tokens.size())
        {
            return false;
        }

        if (tokens[idx].type == TokenType::kConst)
        {
            idx++;
        }

        if (idx >= tokens.size() || tokens[idx].type != TokenType::kIdentifier)
        {
            return false;
        }

        auto parseTypePath = [&]() -> bool
        {
            idx++;
            while (idx < tokens.size())
            {
                const auto type = tokens[idx].type;
                if (type != TokenType::kDot && type != TokenType::kDoubleColon)
                {
                    break;
                }
                idx++;
                if (idx >= tokens.size() || tokens[idx].type != TokenType::kIdentifier)
                {
                    return false;
                }
                idx++;
            }
            return true;
        };

        if (!parseTypePath())
        {
            return false;
        }

        if (idx < tokens.size() && tokens[idx].type == TokenType::kLt)
        {
            i32 depth = 0;
            while (idx < tokens.size())
            {
                switch (tokens[idx].type)
                {
                case TokenType::kLt:
                    depth++;
                    idx++;
                    break;
                case TokenType::kGt:
                    depth--;
                    idx++;
                    break;
                case TokenType::kShr:
                    depth -= 2;
                    idx++;
                    break;
                case TokenType::kIdentifier:
                case TokenType::kDot:
                case TokenType::kDoubleColon:
                case TokenType::kComma:
                case TokenType::kColon:
                case TokenType::kMul:
                case TokenType::kLBracket:
                case TokenType::kRBracket:
                case TokenType::kIntLit:
                    idx++;
                    break;
                default:
                    return false;
                }

                if (depth <= 0)
                {
                    break;
                }
            }

            if (depth != 0)
            {
                return false;
            }
        }

        while (idx < tokens.size() && tokens[idx].type == TokenType::kMul)
        {
            idx++;
        }

        while (idx < tokens.size() && tokens[idx].type == TokenType::kLBracket)
        {
            idx++;
            while (idx < tokens.size() && tokens[idx].type != TokenType::kRBracket)
            {
                if (tokens[idx].type != TokenType::kIntLit && tokens[idx].type != TokenType::kComma)
                {
                    return false;
                }
                idx++;
            }
            if (idx >= tokens.size() || tokens[idx].type != TokenType::kRBracket)
            {
                return false;
            }
            idx++;
        }

        if (endIdx)
        {
            *endIdx = idx;
        }
        return true;
    }

    // top-statement parser
    // import parser + decls parser
    Expected<void> NParser::parseRoot()
    {
        auto &output = m_args.output;

        do
        {
            if (check(TokenType::kImport))
            {
                const auto start = current();
                auto p_import_Ret = parseImport();
                if (!p_import_Ret)
                {
                    synchronizeTopLevel();
                    output.Nodes.push_back(makeErrorDecl(start));
                    if (shouldAbort())
                    {
                        return p_import_Ret.result();
                    }
                    continue;
                }
                output.Nodes.push_back(p_import_Ret.value());
            }
            else if (check(TokenType::kEOF))
            {
                advance();
                break;
            }
            else
            {
                const auto start = current();
                auto r = parseDecl();
                if (!r)
                {
                    synchronizeTopLevel();
                    if (current() == start && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    output.Nodes.push_back(makeErrorDecl(start));
                    if (shouldAbort())
                    {
                        return r.result();
                    }
                    continue;
                }
                if (r.value() == nullptr)
                {
                    m_diag.error(start.location(m_args.file), "unexpected token " + tokenText(start) + " in top-level declaration");
                    synchronizeTopLevel();
                    if (current() == start && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    output.Nodes.push_back(makeErrorDecl(start));
                    if (shouldAbort())
                    {
                        return Result::failure("too many errors, aborting parse");
                    }
                    continue;
                }
                output.Nodes.push_back(r.value());
            }
        } while (true);

        return Result::success();
    }

    // import statement parser
    // suppoting module string lit like "aaa.bbb.ccc"
    // TESTED
    Expected<ImportDecl *> NParser::parseImport()
    {
        auto start = current();
        if (!check(TokenType::kImport))
        {
            return unexpectedToken("import declaration");
        }
        String moduleName{};

        // parse import string lit, module names concat with dot
        do
        {
            advance();
            if (check(TokenType::kIdentifier))
            {
                moduleName.append(current().value);
            }
            else if (check(TokenType::kDot))
            {
                moduleName.append(".");
            }
            else if (check(TokenType::kSemicolon))
            {
                advance();
                break;
            }
            else
            {
                return unexpectedToken("module path");
            }
        } while (true);

        return setLoc(neo::newObject<ImportDecl>(moduleName), start);
    }

    // module declare parser
    // support syntax like "module aaa;" or "module bbb {...}"
    // TESTED
    Expected<ModuleDecl *> NParser::parseModule()
    {
        auto start = current();
        if (!check(TokenType::kModule))
        {
            return nullptr;
        }
        String module{};

        // parse module name decl
        // support syntax like 'aaa' or 'aaa.bbb'
        // end at ';' or '{'
        do
        {
            advance();
            if (check(TokenType::kIdentifier))
            {
                module.append(current().value);
            }
            else if (check(TokenType::kDot))
            {
                module.append(".");
            }
            else if (check(TokenType::kSemicolon) || check(TokenType::kLBraces))
            {
                break;
            }
            else
            {
                return unexpectedToken("module declaration");
            }
        } while (true);
        auto gd = ScopeGuard(new ModuleDecl(module));

        if (check(TokenType::kSemicolon))
        {
            // top level module declaration with no body
            advance(); // Skip ';'
        }
        else if (check(TokenType::kLBraces))
        {
            // scope-based module decl
            // trigger scope decl parsing logic and make those decls as module's children
            auto children = ScopeGuard(new TopLevelDecls());
            setLoc(children.getPtr(), current());
            advance(); // eat '{'

            do
            {
                if (check(TokenType::kRBraces))
                {
                    advance(); // eat '}'
                    break;
                }
                else
                {
                    auto r = parseDecl();
                    CHECK_ERROR(r);
                    //                    gd->children->decls.push_back(r.value());
                    children->decls.push_back(r.value());
                }
            } while (true);
            gd->children = children.getPtr();
        }
        else
        {
            return expectedToken("';' or '{'", "module declaration");
        }

        return setLoc(gd.getPtr(), start);
    }

    // function declaration parser
    // syntax like xxx fun xxx(...) xxx {...}
    // TESTED
    Expected<FuncDecl *> NParser::parseFunc(bool isLambda)
    {
        const auto start = current();
        const bool isCtor = check(TokenType::kCtor);
        const bool isDtor = check(TokenType::kDtor);
        
        if (!check(TokenType::kFun) && !isCtor && !isDtor)
        {
            return unexpectedToken("function declaration");
        }
        if (!isLambda && !isCtor && !isDtor && !expect(TokenType::kIdentifier))
        {
            return expectedToken("a function name", "function declaration");
        }
        String name{};
        advance(); // Skip name label

        InitialStmt *initStmt = nullptr;
        Vector<GenericParamDecl *> genericParams;

        if (!isLambda)
        {
            if (isCtor || isDtor)
            {
                name = start.value;
            }
            else
            {
                // function name parsing logic
                name = current().value;
                advance(); // Skip name

                String genericSuffix{};
                auto rGeneric = parseGenericSuffix(genericSuffix, &genericParams);
                CHECK_ERROR(rGeneric);
            }
        }

        if (!check(TokenType::kLParen))
        {
            return expectedToken("'('", "function parameter list");
        }

        // function argument parsing
        auto oldGenericTypeNames = std::move(m_activeGenericTypeNames);
        m_activeGenericTypeNames = Vector<String>{};
        for (auto *param : genericParams)
        {
            if (param != nullptr)
            {
                m_activeGenericTypeNames.push_back(param->name);
            }
        }
        auto args = parseFuncArgs();
        m_activeGenericTypeNames = std::move(oldGenericTypeNames);
        CHECK_ERROR(args);
        for (auto *arg : args.value())
        {
            if (arg && arg->m_loc.file == nullptr)
            {
                arg->m_loc = start.location(m_args.file);
            }
        }

        ASTTypeNode *returnType = nullptr;

        parseMain:
        // check scope-based decl or interface-based decl
        if (check(TokenType::kSemicolon))
        {
            // end with ';' just return
            advance();
            if (!isCtor && !isDtor && returnType == nullptr)
            {
                returnType = setLoc(new ASTTypeNode("void"), start);
            }
            return setLoc(new FuncDecl(name, std::move(genericParams), returnType, args.value(), initStmt, nullptr), start);
        }
        if ((isCtor || isDtor) && check(TokenType::kColon)) 
        {
            // parse initial statements.
            if (!isCtor && !isDtor) {
                return unexpectedToken("initial statement can't use on general functions");
            }
            advance(); // Skip ':'
            initStmt = setLoc(new InitialStmt(), current());

            do {
                if (check(TokenType::kComma)) {
                    advance();
                    continue;
                } else if (check(TokenType::kIdentifier)) {
                    auto label = current();
                    advance(); // Skip variable label
                    if (!check(TokenType::kLParen)) {
                        return expectedToken("'('", "initial statement value expression.");
                        break;
                    }
                    auto args = parseFuncCallArgs();
                    CHECK_ERROR(args);
                    auto* callee = setLoc(new ASTIdent(label.value), label);
                    auto* oExpr = setLoc(new CallExpr(callee, args.value()), label);
                    initStmt->initialStmts.push_back(oExpr);
                } else if (check(TokenType::kLBraces) || check(TokenType::kSemicolon)) {
                    break;
                } else {
                    return unexpectedToken("initial statements");
                }
            } while(true);
            goto parseMain; // Recheck again and try parse main body.
        }
        // function return type parsing (non-ctor/dtor)
        if (!isCtor && !isDtor)
        {
            if (check(TokenType::kColon))
            {
                advance(); // Skip ':'
                auto t = parseType();
                CHECK_ERROR(t);
                if (t.value() == nullptr)
                {
                    return expectedToken("a return type", "function declaration");
                }
                returnType = t.value();
            }
            else if (check(TokenType::kIdentifier))
            {
                auto t = parseType();
                CHECK_ERROR(t);
                returnType = t.value();
            }
        }
        if (check(TokenType::kLBraces))
        {
            // end with '{'

            auto r = parseScope();
            CHECK_ERROR(r);
            if (!isCtor && !isDtor && returnType == nullptr)
            {
                returnType = setLoc(new ASTTypeNode("void"), start);
            }
            return setLoc(new FuncDecl(name, std::move(genericParams), returnType, args.value(), initStmt, r.value()), start);
        }
        else
        {
            return expectedToken("';' or '{'", "function declaration");
        }

        return Result::failure("internal parser error while finishing function declaration", ERRR());
    }

    Expected<void> NParser::parseGenericSuffix(String &out, Vector<GenericParamDecl *> *params)
    {
        if (!check(TokenType::kLt))
        {
            return Result::success();
        }

        if (params != nullptr)
        {
            const auto genericStart = current();
            bool terminated = false;
            advance(); // Skip '<'

            do
            {
                if (check(TokenType::kGt))
                {
                    advance(); // Skip '>'
                    terminated = true;
                    break;
                }

                if (!check(TokenType::kIdentifier))
                {
                    m_diag.error(current().location(m_args.file), "expected generic parameter name");
                    while (!check(TokenType::kEOF) && !check(TokenType::kComma) && !check(TokenType::kGt))
                    {
                        advance();
                    }
                    if (check(TokenType::kComma))
                    {
                        advance();
                        continue;
                    }
                    if (check(TokenType::kGt))
                    {
                        advance();
                        terminated = true;
                        break;
                    }
                    break;
                }

                const auto paramStart = current();
                ASTTypeNode *constraint = nullptr;
                String name = current().value;
                advance(); // Skip parameter name

                if (check(TokenType::kColon))
                {
                    advance(); // Skip ':'
                    auto rConstraint = parseType();
                    if (!rConstraint)
                    {
                        while (!check(TokenType::kEOF) && !check(TokenType::kComma) && !check(TokenType::kGt))
                        {
                            advance();
                        }
                        m_diag.error(current().location(m_args.file), "expected generic constraint type");
                    }
                    else if (rConstraint.value() == nullptr)
                    {
                        m_diag.error(current().location(m_args.file), "expected generic constraint type");
                    }
                    else
                    {
                        constraint = rConstraint.value();
                    }
                }

                auto *param = setLoc(neo::newObject<GenericParamDecl>(name, constraint), paramStart);
                params->push_back(param);

                if (check(TokenType::kComma))
                {
                    advance(); // Skip ','
                    continue;
                }
                if (check(TokenType::kGt))
                {
                    advance(); // Skip '>'
                    terminated = true;
                    break;
                }

                m_diag.error(current().location(m_args.file), "expected ',' or '>' in generic parameter list");
                while (!check(TokenType::kEOF) && !check(TokenType::kComma) && !check(TokenType::kGt))
                {
                    advance();
                }
                if (check(TokenType::kComma))
                {
                    advance();
                    continue;
                }
                if (check(TokenType::kGt))
                {
                    advance();
                    terminated = true;
                    break;
                }
                break;
            } while (!check(TokenType::kEOF));

            if (!terminated)
            {
                return Result::failure("unterminated generic parameter list", &m_diag, genericStart, m_args.file);
            }

            return Result::success();
        }

        const auto start = current();
        i32 depth = 0;
        do
        {
            if (check(TokenType::kLt))
            {
                depth++;
            }
            else if (check(TokenType::kGt))
            {
                depth--;
            }
            else if (check(TokenType::kShr))
            {
                out.append(current().value);
                depth -= 2;
                advance();
                if (depth <= 0)
                {
                    break;
                }
                continue;
            }

            out.append(current().value);
            advance();
        } while (depth > 0 && !check(TokenType::kEOF));

        if (depth != 0)
        {
            return Result::failure("unterminated generic parameter list", &m_diag, start, m_args.file);
        }

        if (params)
        {
            // Re-parse the collected suffix for generic params: <T, U: Base>
            // We can read from 'out' to fill params without changing token stream.
            // For now, keep it minimal: only names and optional single-type constraint.
            StringView view{out};
            psize idx = 0;
            while (idx < view.size())
            {
                if (view[idx] == '<' || view[idx] == '>' || view[idx] == ',' || view[idx] == ' ')
                {
                    idx++;
                    continue;
                }
                // read name
                psize startIdx = idx;
                while (idx < view.size() && (std::isalnum(static_cast<unsigned char>(view[idx])) || view[idx] == '_'))
                {
                    idx++;
                }
                String name{view.data() + startIdx, idx - startIdx};
                ASTTypeNode *constraint = nullptr;

                // skip spaces
                while (idx < view.size() && view[idx] == ' ')
                {
                    idx++;
                }
                if (idx < view.size() && view[idx] == ':')
                {
                    idx++;
                    while (idx < view.size() && view[idx] == ' ')
                    {
                        idx++;
                    }
                    psize cStart = idx;
                    while (idx < view.size() && (std::isalnum(static_cast<unsigned char>(view[idx])) || view[idx] == '_' || view[idx] == '.' || view[idx] == ':'))
                    {
                        idx++;
                    }
                    String cName{view.data() + cStart, idx - cStart};
                    if (!cName.empty())
                    {
                        constraint = neo::newObject<ASTTypeNode>(cName);
                    }
                }
                if (!name.empty())
                {
                    params->push_back(neo::newObject<GenericParamDecl>(name, constraint));
                }
            }
        }

        return Result::success();
    }

    // common type paring function
    // dealing normal type & array type & pointer type
    // TESTED
    Expected<ASTTypeNode *> NParser::parseType()
    {
        const auto start = current();
        bool isConst = false;
        if (check(TokenType::kConst) || (check(TokenType::kIdentifier) && current().value == "const"))
        {
            isConst = true;
            advance();
        }

        if (!check(TokenType::kIdentifier))
        {
            if (isConst)
            {
                return expectedToken("type identifier", "type");
            }
            return nullptr;
        }

        // get full type string including module and type
        String typeStr;
        if (isConst)
        {
            typeStr.append("const ");
        }
        typeStr.append(current().value);
        advance();

        while (check(TokenType::kDot) || check(TokenType::kDoubleColon))
        {
            String sep = current().type == TokenType::kDoubleColon ? "::" : ".";
            advance(); // eat separator

            if (!check(TokenType::kIdentifier))
            {
                return expectedToken("an identifier", "qualified type name");
            }

            typeStr.append(sep);
            typeStr.append(current().value);
            advance();
        }

        auto rGeneric = parseGenericSuffix(typeStr, nullptr);
        CHECK_ERROR(rGeneric);

        bool hasPointer = false;
        while (check(TokenType::kMul))
        {
            hasPointer = true;
            typeStr.append("*");
            advance();
        }

        if (check(TokenType::kLBracket))
        {
            // parse array type's bracket and check array dimenssion

            ScopeGuard<ASTArrayType> gd{new ASTArrayType(std::move(typeStr), false, {})};
            do
            {
                advance(); // eat left bracket '['
                do
                {
                    if (check(TokenType::kIntLit))
                    {
                        gd->size.push_back(std::stoi(current().value.data()));
                        advance();
                        continue;
                    }
                    else if (check(TokenType::kComma))
                    {
                        advance();
                        continue;
                    }
                    else if (check(TokenType::kRBracket))
                    {
                        advance();
                        break;
                    }
                    else
                    {
                        return Result::failure("expected ']' to close array type", ERRR());
                    }
                } while (true);
            } while (check(TokenType::kLBracket));
            gd->dimenssion = (i32)gd->size.size();
            if (gd->size.empty())
            {
                gd->isReceiver = true;
            }
            return setLoc(gd.getPtr(), start);
        }
        else if (hasPointer)
        {
            return setLoc(new ASTPointerType(std::move(typeStr)), start);
        }
        else
        {
            return setLoc(new ASTTypeNode(std::move(typeStr)), start);
        }
    }

    // modifier parser
    // support all modifier register in s_modifier
    // TESTED
    Expected<ASTModifier> NParser::parseModifier()
    {
        ASTModifier mf{};

        do
        {
            TokenType curType = current().type;
            if (curType == TokenType::kIdentifier)
            {
                const auto &val = current().value;
                if (val == "private") curType = TokenType::kPrivate;
                else if (val == "protected") curType = TokenType::kProtected;
                else if (val == "internal") curType = TokenType::kInternal;
                else if (val == "inline") curType = TokenType::kInline;
                else if (val == "static") curType = TokenType::kStatic;
                else if (val == "const") curType = TokenType::kConst;
                else if (val == "virtual") curType = TokenType::kVirtual;
                else if (val == "override") curType = TokenType::kOverride;
                else if (val == "impl") curType = TokenType::kImpl;
                else if (val == "final") curType = TokenType::kFinal;
            }

            if (std::find(std::begin(s_modifier), std::end(s_modifier), curType) == std::end(s_modifier))
            {
                break;
            }

            switch (curType)
            {
            case TokenType::kPrivate:
                CHECK_MODIFIER(mf.isPrivate, "private")
                mf.isPrivate = true;
                break;
            case TokenType::kProtected:
                CHECK_MODIFIER(mf.isProtected, "protected")
                mf.isProtected = true;
                break;
            case TokenType::kInternal:
                CHECK_MODIFIER(mf.isInternal, "internal")
                mf.isInternal = true;
                break;
            case TokenType::kInline:
                CHECK_MODIFIER(mf.isInline, "inline")
                mf.isInline = true;
                break;
            case TokenType::kStatic:
                CHECK_MODIFIER(mf.isStatic, "static")
                mf.isStatic = true;
                break;
            case TokenType::kConst:
                CHECK_MODIFIER(mf.isConst, "const")
                mf.isConst = true;
                break;
            case TokenType::kVirtual:
                CHECK_MODIFIER(mf.isVirtual, "virtual");
                mf.isVirtual = true;
                break;
            case TokenType::kOverride:
                CHECK_MODIFIER(mf.isOverride, "override");
                mf.isOverride = true;
                break;
            case TokenType::kFinal:
                CHECK_MODIFIER(mf.isFinal, "final");
                mf.isFinal = true;
                break;
            case TokenType::kImpl:
                CHECK_MODIFIER(mf.isImpl, "impl");
                mf.isImpl = true;
                break;
            default:
                break;
            }

            advance();
        } while (true);

        return mf;
    }

    // fuction calling expression's argument list parser
    // syntax like xxx(aa,bb,cc,...)
    //
    Expected<Vector<ASTExpr *>> NParser::parseFuncCallArgs()
    {
        Vector<ASTExpr *> args{};
        if (!check(TokenType::kLParen))
        {
            return args;
        }
        advance(); // Skip '('

        do
        {
            if (check(TokenType::kComma))
            {
                advance(); // Skip ','
            }
            else if (check(TokenType::kRParen))
            {
                advance(); // Skip ')'
                break;
            }
            else
            {
                auto r = parseExpr(true);
                CHECK_ERROR(r);
                args.push_back(r.value());
            }
        } while (true);

        return args;
    }

    // function's argument parser
    // syntax like (xx : xx, xx : xx = xx, ...)
    Expected<Vector<VarDecl *>> NParser::parseFuncArgs()
    {
        // function argument parsing logic;

        Vector<VarDecl *> args{};
        Vector<Attribute *> attrs{};
        auto isActiveGenericTypeName = [&](const String &name) -> bool
        {
            for (const auto &genericName : m_activeGenericTypeNames)
            {
                if (genericName == name)
                {
                    return true;
                }
            }
            return false;
        };
        enum class ParamSyncResult
        {
            kComma,
            kRParen,
            kRBrace,
            kEOF
        };
        auto syncParam = [&]() -> ParamSyncResult {
            while (!check(TokenType::kEOF))
            {
                if (check(TokenType::kComma))
                {
                    advance();
                    return ParamSyncResult::kComma;
                }
                if (check(TokenType::kRParen))
                {
                    advance();
                    return ParamSyncResult::kRParen;
                }
                if (check(TokenType::kRBraces))
                {
                    return ParamSyncResult::kRBrace;
                }
                advance();
            }
            return ParamSyncResult::kEOF;
        };

        if (!check(TokenType::kLParen))
            return args;
        advance(); // Skip '('
        do
        {
            if (check(TokenType::kEOF))
            {
                m_diag.hint(previous().location(m_args.file), "insert ')' to finish the parameter list");
                return Result::failure("expected ')' to close function parameter list", ERRR());
            }
            if (check(TokenType::kRBraces))
            {
                m_diag.hint(previous().location(m_args.file), "insert ')' before the block starts");
                return Result::failure("expected ')' to close function parameter list", ERRR());
            }
            if (check(TokenType::kRParen))
            {
                advance();
                break;
            }
            if (check(TokenType::kLBracket))
            {
                // parse attributes

                auto att = parseAttributes();
                if (!att)
                {
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }
                attrs = att.value();
            }
            auto r = parseModifier();
            CHECK_ERROR(r);
            auto md = r.value();

            if ((check(TokenType::kIdentifier) || check(TokenType::kVal) || check(TokenType::kVar)) && expect(TokenType::kColon))
            {
                const auto argStart = current();

                String arg_name = current().value;
                advance();
                if (!check(TokenType::kColon))
                {
                    m_diag.error(current().location(m_args.file), "expected ':' after function parameter name");
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }
                advance(); // Skip ':'

                auto t = parseType();
                if (!t)
                {
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }
                if (t.value() == nullptr)
                {
                    m_diag.error(current().location(m_args.file), "expected parameter type");
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }

                if (check(TokenType::kAssign))
                {
                    advance();
                    auto epr = parseExpr();
                    CHECK_ERROR(epr);
                    args.push_back(new VarDecl(arg_name, t.value(), epr.value()));
                    args.back()->m_loc = argStart.location(m_args.file);
                    args.back()->attributes = std::move(attrs);
                    args.back()->modifier = std::move(md);
                    attrs = Vector<Attribute *>{};

                    continue;
                }
                else if (check(TokenType::kComma) || check(TokenType::kRParen))
                {
                    args.push_back(new VarDecl(arg_name, t.value()));
                    args.back()->m_loc = argStart.location(m_args.file);
                    args.back()->attributes = std::move(attrs);
                    args.back()->modifier = std::move(md);
                    attrs = Vector<Attribute *>{};

                    // only break when meet ')'
                    if (check(TokenType::kRParen))
                    {
                        advance();
                        break;
                    }
                    advance();
                    continue;
                }
                else
                {
                    m_diag.error(current().location(m_args.file), "unexpected token after function parameter declaration");
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }
            }
            else if (check(TokenType::kIdentifier) && isActiveGenericTypeName(current().value) && peek().type == TokenType::kIdentifier)
            {
                const auto argStart = current();
                auto rType = parseType();
                if (!rType)
                {
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }

                String arg_name = current().value;
                advance();

                if (check(TokenType::kAssign))
                {
                    advance();
                    auto epr = parseExpr();
                    CHECK_ERROR(epr);
                    args.push_back(new VarDecl(arg_name, rType.value(), epr.value()));
                }
                else
                {
                    args.push_back(new VarDecl(arg_name, rType.value()));
                }
                args.back()->m_loc = argStart.location(m_args.file);
                args.back()->attributes = std::move(attrs);
                args.back()->modifier = std::move(md);
                attrs = Vector<Attribute *>{};

                if (check(TokenType::kRParen))
                {
                    advance();
                    break;
                }
                if (!check(TokenType::kComma))
                {
                    m_diag.error(current().location(m_args.file), "expected ',' or ')' in function argument list");
                    auto sr = syncParam();
                    attrs = Vector<Attribute *>{};
                    if (sr != ParamSyncResult::kComma)
                    {
                        break;
                    }
                    continue;
                }
                advance();
                continue;
            }
            else if (check(TokenType::kComma))
            {
                // skip comma

                advance(); // Skip ','
                continue;
            }
            else if (check(TokenType::kRParen))
            {
                // end loop when matched ')'

                advance(); // Skip ')'
                break;
            }
            else
            {
                m_diag.error(current().location(m_args.file), "unexpected token " + tokenText(current()) + " in function argument list");
                auto sr = syncParam();
                attrs = Vector<Attribute *>{};
                if (sr != ParamSyncResult::kComma)
                {
                    break;
                }
                continue;
            }
        } while (true);

        return args;
    }

    // scope statments parser
    // syntax like { ... }
    Expected<CompoundStmt *> NParser::parseScope()
    {
        if (!check(TokenType::kLBraces))
        {
            return nullptr;
        }
        const auto scopeStart = current();
        advance(); // eat '{'
        const auto start = current();
        auto gd = ScopeGuard(new CompoundStmt());

        do
        {
            if (check(TokenType::kRBraces))
            {
                advance(); // eat '}'
                break;
            }
            else
            {
                const auto stmtStart = current();
                if (stmtStart.type == TokenType::kEOF)
                {
                    m_diag.hint(scopeStart.location(m_args.file), "insert '}' to match this opening brace");
                    return Result::failure("expected '}' to close scope", &m_diag, scopeStart, m_args.file);
                }
                auto r = parseStmt();
                if (!r)
                {
                    synchronizeStmt();
                    if (current() == stmtStart && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    gd->statements.push_back(makeErrorStmt(stmtStart));
                    if (shouldAbort())
                    {
                        return r.result();
                    }
                    continue;
                }
                if (r.value() == nullptr)
                {
                    synchronizeStmt();
                    if (current() == stmtStart && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    gd->statements.push_back(makeErrorStmt(stmtStart));
                    if (shouldAbort())
                    {
                        return Result::failure("too many errors, aborting parse");
                    }
                    continue;
                }
                gd->statements.push_back(r.value());
            }
        } while (true);

        return setLoc(gd.getPtr(), start);
    }

    // parse attributes on decls
    // syntax like [xxx(...)] or [xxx]
    Expected<Vector<Attribute *>> NParser::parseAttributes()
    {
        Vector<Attribute *> attrs{};

        do
        {
            // check '[xxx'
            if (!(check(TokenType::kLBracket) && expect(TokenType::kIdentifier)))
            {
                return attrs;
            }
            advance(); // Skip '['

            // get attribute's string-lit
            String name = current().value;
            ScopeGuard g{new Attribute{}};
            g->name = name;
            advance(); // Skip name

            // check '[xxx(' <- and parse args
            if (check(TokenType::kRBracket))
            {
                attrs.push_back(g.getPtr());
                advance(); // Skip ']'
                continue;
            }
            else if (check(TokenType::kLParen))
            {
                // parse attribute's arguments

                auto r = parseFuncCallArgs();
                CHECK_ERROR(r);
                g->arguments.swap(r.value());
                if (!check(TokenType::kRBracket))
                {
                    CLEARUP(attrs);
                    m_diag.hint(g->arguments.empty() ? current().location(m_args.file) : g->arguments.back()->m_loc, "insert ']' after the attribute arguments");
                    return Result::failure(msg("expected ']' to close attribute, found ", tokenText(current())), ERRR());
                }
                advance(); // Skip ']'
                attrs.push_back(g.getPtr());
            }
            else
            {
                CLEARUP(attrs);
                return Result::failure(msg("unexpected token ", tokenText(current()), " after attribute name"), ERRR());
            }
        } while (check(TokenType::kLBracket));

        return attrs;
    }

    // declaration parser
    // for function decl/class decl/struct decl/module decl...
    Expected<ASTDecl *> NParser::parseDecl()
    {
        auto md = parseModifier();
        CHECK_ERROR(md);
        Vector<Attribute *> attrs{};
        auto isRecoverableDeclToken = [&](TokenType t) -> bool {
            return t == TokenType::kFun || t == TokenType::kClass || t == TokenType::kStruct ||
                   t == TokenType::kInterface || t == TokenType::kEnum || t == TokenType::kModule ||
                   t == TokenType::kVar || t == TokenType::kVal || isModifier(t);
        };
        auto mergeModifier = [&](ASTModifier &dst, const ASTModifier &src) -> Result {
            if (src.isStatic && dst.isStatic) return Result::failure("duplicated modifier static", ERRR());
            if (src.isFinal && dst.isFinal) return Result::failure("duplicated modifier final", ERRR());
            if (src.isConst && dst.isConst) return Result::failure("duplicated modifier const", ERRR());
            if (src.isPrivate && dst.isPrivate) return Result::failure("duplicated modifier private", ERRR());
            if (src.isProtected && dst.isProtected) return Result::failure("duplicated modifier protected", ERRR());
            if (src.isInternal && dst.isInternal) return Result::failure("duplicated modifier internal", ERRR());
            if (src.isInline && dst.isInline) return Result::failure("duplicated modifier inline", ERRR());
            if (src.isVirtual && dst.isVirtual) return Result::failure("duplicated modifier virtual", ERRR());
            if (src.isOverride && dst.isOverride) return Result::failure("duplicated modifier override", ERRR());
            if (src.isImpl && dst.isImpl) return Result::failure("duplicated modifier impl", ERRR());

            dst.isStatic |= src.isStatic;
            dst.isFinal |= src.isFinal;
            dst.isConst |= src.isConst;
            dst.isPrivate |= src.isPrivate;
            dst.isProtected |= src.isProtected;
            dst.isInternal |= src.isInternal;
            dst.isInline |= src.isInline;
            dst.isVirtual |= src.isVirtual;
            dst.isOverride |= src.isOverride;
            dst.isImpl |= src.isImpl;
            return Result::success();
        };

        if (check(TokenType::kLBracket))
        {
            // parse attribute

            auto p_attribute_Ret = parseAttributes();
            if (!p_attribute_Ret)
            {
                if (!check(TokenType::kEOF) && isRecoverableDeclToken(current().type))
                {
                    attrs.clear();
                }
                else
                {
                    return p_attribute_Ret.result();
                }
            }
            else
            {
            attrs = p_attribute_Ret.value();
            }
        }

        auto md2 = parseModifier();
        CHECK_ERROR(md2);
        auto mr = mergeModifier(md.value(), md2.value());
        if (mr.hasError())
        {
            return mr;
        }
        if (check(TokenType::kModule))
        {
            // module decl parsing logic

            auto p_module_Ret = parseModule();
            CHECK_ERROR(p_module_Ret);
            APPLY_MODIFIER(p_module_Ret, md);
            APPLY_ATTRIBUTES(p_module_Ret, attrs);
            return p_module_Ret.value();
        }
        else if (check(TokenType::kFun))
        {
            // function decl parsing logic

            auto p_func_Ret = parseFunc();
            CHECK_ERROR(p_func_Ret);
            APPLY_MODIFIER(p_func_Ret, md);
            APPLY_ATTRIBUTES(p_func_Ret, attrs);
            return p_func_Ret.value();
        }
        else if (check(TokenType::kClass))
        {
            // class decl parsing logic

            auto p_class_Ret = parseClass();
            CHECK_ERROR(p_class_Ret);
            APPLY_MODIFIER(p_class_Ret, md);
            APPLY_ATTRIBUTES(p_class_Ret, attrs);
            return p_class_Ret.value();
        }
        else if (check(TokenType::kStruct))
        {
            // struct decl parsing logic

            auto p_struct_Ret = parseStruct();
            CHECK_ERROR(p_struct_Ret);
            APPLY_MODIFIER(p_struct_Ret, md);
            APPLY_ATTRIBUTES(p_struct_Ret, attrs);
            return p_struct_Ret.value();
        }
        else if (check(TokenType::kInterface))
        {
            // interfacce decl parsing logic

            auto p_intf_Ret = parseInterface();
            CHECK_ERROR(p_intf_Ret);
            APPLY_MODIFIER(p_intf_Ret, md);
            APPLY_ATTRIBUTES(p_intf_Ret, attrs);
            return p_intf_Ret.value();
        }
        else if (check(TokenType::kEnum))
        {
            // enum decl parsing logic

            auto p_enum_Ret = parseEnum();
            CHECK_ERROR(p_enum_Ret);
            APPLY_MODIFIER(p_enum_Ret, md);
            APPLY_ATTRIBUTES(p_enum_Ret, attrs);
            return p_enum_Ret.value();
        }
        else if (check(TokenType::kVal) || check(TokenType::kVar))
        {
            // variable parsing

            auto p_var_Ret = parseVarDecl();
            CHECK_ERROR(p_var_Ret);
            APPLY_MODIFIER(p_var_Ret, md);
            APPLY_ATTRIBUTES(p_var_Ret, attrs);
            return p_var_Ret.value();
        }
        else
        {
            return nullptr;
        }
    }

    // class parser
    // syntax like [xxx(xxx)] xxx class xxx : xxx { ... }
    Expected<ClassDecl *> NParser::parseClass()
    {
        const auto start = current();
        if (!check(TokenType::kClass))
        {
            return nullptr;
        }
        advance();

        // class name parsing
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected class name, found ", tokenText(current())), ERRR());
        }
        String name = current().value;
        advance();

        // super classes parsing
        auto gd = ScopeGuard<ClassDecl>(new ClassDecl(name, {}));

        // pre-def for body parsing
        Vector<Attribute *> attrs{};
        ASTModifier md{};
        auto isClassMemberStartToken = [&](TokenType t) -> bool
        {
            return t == TokenType::kLBracket || t == TokenType::kFun || t == TokenType::kCtor ||
                   t == TokenType::kDtor || t == TokenType::kField || t == TokenType::kVar ||
                   t == TokenType::kVal || t == TokenType::kClass || t == TokenType::kStruct ||
                   t == TokenType::kInterface || t == TokenType::kEnum || isModifier(t);
        };
        auto syncMember = [&]() {
            i32 braceDepth = 0;
            while (!check(TokenType::kEOF))
            {
                if (check(TokenType::kLBraces))
                {
                    braceDepth++;
                    advance();
                    continue;
                }
                if (check(TokenType::kRBraces))
                {
                    if (braceDepth == 0)
                    {
                        if (isClassMemberStartToken(peek().type))
                        {
                            advance();
                            continue;
                        }
                        return;
                    }
                    braceDepth--;
                    advance();
                    continue;
                }
                if (braceDepth == 0 &&
                    isClassMemberStartToken(current().type))
                {
                    return;
                }
                advance();
            }
        };

        if (check(TokenType::kColon))
        {
            // parse base class types

            auto pR = parseParents();
            CHECK_ERROR(pR);
            gd->baseClasses = std::move(pR.value());

            if (check(TokenType::kLBraces))
            {
                goto parseBody;
            }
            else if (check(TokenType::kSemicolon))
            {
                goto end;
            }
            else
            {
                return Result::failure(msg("unexpected token ", tokenText(current()), " after class declaration"), ERRR());
            }
        }
        else if (check(TokenType::kLBraces))
        {
            goto parseBody;
        }
        else if (check(TokenType::kSemicolon))
        {
            goto end;
        }
        else
        {
            return Result::failure(msg("unexpected token ", tokenText(current()), " after class declaration"), ERRR());
        }

    parseBody:
        // class body parsing
        advance();

        do
        {
            if (check(TokenType::kLBracket))
            {
                // attributes parsing

                auto rq = parseAttributes();
                CHECK_ERROR(rq);
                attrs = rq.value();
            }
            else if (isModifier(current().type))
            {
                // modifier parsing

                auto r = parseModifier();
                CHECK_ERROR(r);
                md = r.value();
            }
            else if (check(TokenType::kFun) || check(TokenType::kDtor) || check(TokenType::kCtor))
            {
                // class constructor / destructor / normal function parsing

                byte type = check(TokenType::kCtor) ? 1 : (check(TokenType::kDtor) ? 2 : 0);
                const auto memberStart = current();
                auto r = parseFunc();
                if (!r)
                {
                    gd->errorMembers.push_back(makeErrorDecl(memberStart));
                    syncMember();
                    attrs = Vector<Attribute *>{};
                    md = ASTModifier{};
                    if (check(TokenType::kRBraces))
                    {
                        continue;
                    }
                    if (current() == memberStart && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    continue;
                }
                auto *func = r.value();
                APPLY_MODIFIER_RAW(func, md);
                md = ASTModifier{};
                if (func != nullptr)
                {
                    func->attributes = std::move(attrs);
                }
                attrs = Vector<Attribute *>();
                if (type == 1)
                {
                    gd->ctors.push_back(func);
                }
                else if (type == 2)
                {
                    if (gd->dtors != nullptr)
                        return Result::failure("destructor is already declared in this class", ERRR());
                    gd->dtors = func;
                }
                else
                {
                    gd->functions.push_back(func);
                }
            }
            else if (check(TokenType::kField))
            {
                // class field parsing

                const auto memberStart = current();
                auto r = parseField();
                if (!r)
                {
                    gd->errorMembers.push_back(makeErrorDecl(memberStart));
                    syncMember();
                    attrs = Vector<Attribute *>{};
                    md = ASTModifier{};
                    if (check(TokenType::kRBraces))
                    {
                        continue;
                    }
                    if (current() == memberStart && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    continue;
                }
                auto *field = r.value();
                APPLY_MODIFIER_RAW(field, md);
                md = ASTModifier{};
                if (field != nullptr)
                {
                    field->attributes = std::move(attrs);
                }
                attrs = Vector<Attribute *>();
                gd->fields.push_back(field);
            }
            else if (check(TokenType::kRBraces))
            {
                // end scope parsed

                advance();
                return setLoc(gd.getPtr(), start);
            }
            else
            {
                const auto memberStart = current();
                auto dr = parseDecl();
                if (!dr)
                {
                    gd->errorMembers.push_back(makeErrorDecl(memberStart));
                    syncMember();
                    if (check(TokenType::kRBraces))
                    {
                        break;
                    }
                    if (current() == memberStart && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    continue;
                }
                if (dr.value() == nullptr)
                {
                    m_diag.error(current().location(m_args.file), "unexpected token " + tokenText(current()) + " in class member declaration");
                    gd->errorMembers.push_back(makeErrorDecl(memberStart));
                    syncMember();
                    if (check(TokenType::kRBraces))
                    {
                        break;
                    }
                    if (current() == memberStart && !check(TokenType::kEOF))
                    {
                        advance();
                    }
                    continue;
                }
                auto *dk = dr.value();
                APPLY_MODIFIER_RAW(dk, md);
                md = ASTModifier{};
                dk->attributes = std::move(attrs);
                attrs = Vector<Attribute *>();

                // sub-data-types
                if (auto *dClass = dynamic_cast<ClassDecl *>(dk))
                {
                    gd->subDataTypes.push_back(dClass);
                }
                else if (auto *dStruct = dynamic_cast<StructDecl *>(dk))
                {
                    gd->subDataTypes.push_back(dStruct);
                }
                else if (auto *dIntf = dynamic_cast<InterfaceDecl *>(dk))
                {
                    gd->subDataTypes.push_back(dIntf);
                }
                else if (auto *dEnum = dynamic_cast<EnumDecl *>(dk))
                {
                    gd->subDataTypes.push_back(dEnum);
                }
                // variabled
                else if (auto *dVar = dynamic_cast<VarDecl *>(dk))
                {
                    gd->variables.push_back(dVar);
                }
                else
                {
                    return Result::failure("unexpected declaration kind in class body", ERRR());
                }
            }
        } while (true);

    end:
        if (check(TokenType::kSemicolon))
        {
            advance();
        }
        return setLoc(gd.getPtr(), start);
    }

    // variable parser
    // syntax like 'var xxx : xxx = xxx' or 'val xxx : xxx = xxx'
    Expected<VarDecl *> NParser::parseVarDecl(bool consumeTerminator)
    {
        const auto start = current();
        if (!check(TokenType::kVal) && !check(TokenType::kVar))
        {
            return nullptr;
        }
        advance();

        // parse variable name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected variable name, found ", tokenText(current())), ERRR());
        }
        StringView name = current().value;
        advance();
        auto gd = ScopeGuard(new VarDecl(name, nullptr));

        if (check(TokenType::kColon))
        {
            // parse type hint

            advance(); // EAT Colon
            auto r = parseType();
            CHECK_ERROR(r);
            gd->type = r.value();
        }

        if (check(TokenType::kAssign))
        {
            advance();
            auto r = parseExpr();
            CHECK_ERROR(r);
            gd->initExpr = r.value();
        }

        if (gd->type == nullptr && gd->initExpr == nullptr)
        {
            return expectedToken("':' or '='", "variable declaration");
        }

        if (consumeTerminator)
        {
            if (!check(TokenType::kSemicolon))
            {
                bool stmtBoundary = check(TokenType::kRBraces) || check(TokenType::kEOF) || check(TokenType::kIf) ||
                                    check(TokenType::kWhile) || check(TokenType::kFor) || check(TokenType::kReturn) ||
                                    check(TokenType::kContinue) || check(TokenType::kTry) || check(TokenType::kThrow) ||
                                    check(TokenType::kBreak) || check(TokenType::kVal) || check(TokenType::kVar) ||
                                    check(TokenType::kFun) || check(TokenType::kClass) || check(TokenType::kStruct) ||
                                    check(TokenType::kInterface) || check(TokenType::kEnum) || check(TokenType::kModule) ||
                                    check(TokenType::kIdentifier) || check(TokenType::kIntLit) || check(TokenType::kFloatLit) ||
                                    check(TokenType::kStringLit) || check(TokenType::kLParen) || check(TokenType::kNew) ||
                                    isModifier(current().type);
                if (stmtBoundary)
                {
                    m_diag.error(current().location(m_args.file), "expected ';' after variable declaration");
                    return setLoc(gd.getPtr(), start);
                }
                return expectedToken("';'", "variable declaration");
            }
            advance();
        }

        return setLoc(gd.getPtr(), start);
    }

    Expected<VarDecl *> NParser::parseTypedVarDecl(bool consumeTerminator)
    {
        psize endIdx = 0;
        if (!isTypeAt(m_lexer->m_tk_idx, &endIdx))
        {
            return nullptr;
        }
        if (endIdx >= m_lexer->m_tokens.size() || m_lexer->m_tokens[endIdx].type != TokenType::kIdentifier)
        {
            return nullptr;
        }

        const auto start = current();
        auto rType = parseType();
        CHECK_ERROR(rType);

        if (!check(TokenType::kIdentifier))
        {
            return Result::failure("expected variable name after type", ERRR());
        }

        StringView name = current().value;
        advance();

        auto gd = ScopeGuard(new VarDecl(name, rType.value()));

        if (check(TokenType::kAssign))
        {
            advance();
            auto rExpr = parseExpr();
            CHECK_ERROR(rExpr);
            gd->initExpr = rExpr.value();
        }

        if (consumeTerminator)
        {
            if (!check(TokenType::kSemicolon))
            {
                return Result::failure("expected ';' after typed variable declaration", ERRR());
            }
            advance();
        }

        return setLoc(gd.getPtr(), start);
    }

    Expected<VarDecl *> NParser::parseColonVarDecl(bool consumeTerminator)
    {
        if (!check(TokenType::kIdentifier) || !expect(TokenType::kColon))
        {
            return nullptr;
        }

        const auto start = current();
        StringView name = current().value;
        advance();

        if (!check(TokenType::kColon))
        {
            return Result::failure("expected ':' after variable name", ERRR());
        }
        advance();

        auto rType = parseType();
        CHECK_ERROR(rType);
        if (rType.value() == nullptr)
        {
            return Result::failure("expected type after ':' in variable declaration", ERRR());
        }

        auto gd = ScopeGuard(new VarDecl(name, rType.value()));

        if (check(TokenType::kAssign))
        {
            advance();
            auto rExpr = parseExpr();
            CHECK_ERROR(rExpr);
            gd->initExpr = rExpr.value();
        }

        if (consumeTerminator)
        {
            if (!check(TokenType::kSemicolon))
            {
                return Result::failure("expected ';' after variable declaration", ERRR());
            }
            advance();
        }

        return setLoc(gd.getPtr(), start);
    }

    // enum parser
    // syntax like 'enum XXX : XXX { ... }'
    Expected<EnumDecl *> NParser::parseEnum()
    {
        const auto start = current();
        if (!check(TokenType::kEnum))
        {
            return nullptr;
        }
        advance();

        // parse enum name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected enum name, found ", tokenText(current())), ERRR());
        }
        StringView name = current().value;
        advance();

        auto gd = ScopeGuard(new EnumDecl(name));

        if (check(TokenType::kColon))
        {
            // parse enum base type

            advance();
            auto t = parseType();
            CHECK_ERROR(t);
            gd->baseType = t.value();

            if (check(TokenType::kLBraces))
            {
                goto parseBody;
            }
            else if (check(TokenType::kSemicolon))
            {
                // head only declare

                advance();
                return setLoc(gd.getPtr(), start);
            }
            else
            {
                return Result::failure(msg("unexpected token ", tokenText(current()), " after enum base type"), ERRR());
            }
        }
        else if (check(TokenType::kLBraces))
        {
            // parse enum body

        parseBody:
            advance(); // Skip '{'
            do
            {
                if (check(TokenType::kRBraces))
                {
                    advance(); // Skip '}'
                    break;
                }
                if (check(TokenType::kComma))
                {
                    advance(); // allow trailing comma
                    continue;
                }
                if (!check(TokenType::kIdentifier))
                {
                    m_diag.error(current().location(m_args.file), "invalid declaration in enum body");
                    synchronize({TokenType::kComma, TokenType::kRBraces});
                    if (check(TokenType::kComma))
                    {
                        advance();
                        continue;
                    }
                    if (check(TokenType::kRBraces))
                    {
                        continue;
                    }
                    break;
                }

                const auto itemStart = current();
                StringView itemName = current().value;
                advance();

                ASTExpr* initExpr = nullptr;
                if (check(TokenType::kAssign))
                {
                    advance();
                    auto eas = parseExpr();
                    CHECK_ERROR(eas);
                    initExpr = eas.value();
                }
                gd->children.push_back(setLoc(new VarDecl(itemName, nullptr, initExpr), itemStart));

                if (check(TokenType::kComma))
                {
                    advance();
                    continue;
                }
                if (check(TokenType::kRBraces))
                {
                    continue;
                }
                m_diag.error(current().location(m_args.file), "invalid declaration in enum body");
                synchronize({TokenType::kComma, TokenType::kRBraces});
                if (check(TokenType::kComma))
                {
                    advance();
                    continue;
                }
                if (check(TokenType::kRBraces))
                {
                    continue;
                }
                break;
            } while (true);
        }
        else
        {
            return Result::failure(msg("unexpected token ", tokenText(current()), " after enum declaration"), ERRR());
        }

        return setLoc(gd.getPtr(), start);
    }

    // field parser
    // syntax like 'field xxx : xxx {XXX,XXX} = xxx;'
    Expected<FieldDecl *> NParser::parseField()
    {
        const auto start = current();
        if (!check(TokenType::kField))
        {
            return nullptr;
        }
        advance();

        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected field name, found ", tokenText(current())), ERRR());
        }
        StringView name = current().value;
        auto gd = ScopeGuard(new FieldDecl(name, nullptr));

        advance();
        if (check(TokenType::kColon))
        {
            // parse type hint

            advance();
            auto thr = parseType();
            CHECK_ERROR(thr);
            gd->type = thr.value();

            // check body
            if (!check(TokenType::kLBraces))
            {
                return Result::failure(msg("expected '{' after field type, found ", tokenText(current())), ERRR());
            }
            goto parseBody;
        }
        else if (check(TokenType::kLBraces))
        {
        parseBody:

            advance();
            // check read function name
            if (check(TokenType::kIdentifier))
            {
                StringView funcName = current().value;
                gd->getFuncName = funcName;
                advance();

                if (!check(TokenType::kComma))
                {
                    return Result::failure(msg("expected ',' after field getter name, found ", tokenText(current())), ERRR());
                }
                advance();
            }
            else if (check(TokenType::kComma))
            {
                advance();
            }
            else
            {
                return Result::failure(msg("unexpected token ", tokenText(current()), " in field accessor list"), ERRR());
            }

            // check write function name
            if (check(TokenType::kIdentifier))
            {
                StringView funcName = current().value;
                gd->setFuncName = funcName;
                advance();

                if (!check(TokenType::kRBraces))
                {
                    return Result::failure(msg("expected '}' after field setter name, found ", tokenText(current())), ERRR());
                }
                advance();
            }
            else if (check(TokenType::kRBraces))
            {
                advance();
            }
            else
            {
                return Result::failure(msg("unexpected token ", tokenText(current()), " in field accessor list"), ERRR());
            }

            if (check(TokenType::kAssign))
            {
                // parse assign expression

                advance();
                auto iexp = parseExpr();
                CHECK_ERROR(iexp);
                gd->init = iexp.value();

                if (!check(TokenType::kSemicolon))
                {
                    goto errorBc;
                }
                advance();
            }
            else if (check(TokenType::kSemicolon))
            {
                advance();
            }
            else if (check(TokenType::kRBraces) || check(TokenType::kLBracket) || isModifier(current().type) ||
                     check(TokenType::kFun) || check(TokenType::kCtor) || check(TokenType::kDtor) ||
                     check(TokenType::kField) || check(TokenType::kClass) || check(TokenType::kStruct) ||
                     check(TokenType::kInterface) || check(TokenType::kEnum) || check(TokenType::kVar) ||
                     check(TokenType::kVal))
            {
                return setLoc(gd.getPtr(), start);
            }
            else
            {
            errorBc:
                return Result::failure(msg("unexpected token ", tokenText(current()), " after field declaration"), ERRR());
            }
        }
        else
        {
            return Result::failure(msg("unexpected token ", tokenText(current()), " after field name"), ERRR());
        }

        return setLoc(gd.getPtr(), start);
    }

    // Statement parser
    // syntax ...
    Expected<ASTStmt *> NParser::parseStmt()
    {
        const auto start = current();
        ASTStmt *result = nullptr;
        auto isStmtRecoveryBoundary = [&]() -> bool
        {
            return check(TokenType::kRBraces) || check(TokenType::kEOF) || check(TokenType::kIf) ||
                   check(TokenType::kWhile) || check(TokenType::kFor) || check(TokenType::kReturn) ||
                   check(TokenType::kContinue) || check(TokenType::kTry) || check(TokenType::kThrow) ||
                   check(TokenType::kBreak) || check(TokenType::kVal) || check(TokenType::kVar) ||
                   check(TokenType::kFun) || check(TokenType::kClass) || check(TokenType::kStruct) ||
                   check(TokenType::kInterface) || check(TokenType::kEnum) || check(TokenType::kModule) ||
                   isModifier(current().type);
        };

        if (check(TokenType::kRBraces) || check(TokenType::kEOF))
        {
            return nullptr;
        }

        switch (current().type)
        {
        case TokenType::kIf:
        {
            // Parse if statement.

            auto rIf = parseIfStmt();
            CHECK_ERROR(rIf);
            result = rIf.value();

            break;
        }
        case TokenType::kWhile:
        {
            // Parse while statement

            auto rWhile = parseWhileStmt();
            CHECK_ERROR(rWhile);
            result = rWhile.value();

            break;
        }
        case TokenType::kFor:
        {
            // Parse for loop statement

            auto rFor = parseForStmt();
            CHECK_ERROR(rFor);
            result = rFor.value();

            break;
        }
        case TokenType::kReturn:
        {
            // Parse return statement.

            auto rRtn = parseReturnStmt();
            CHECK_ERROR(rRtn);
            result = rRtn.value();

            break;
        }
        case TokenType::kContinue:
        {
            // Parse continue statement.
            advance(); // Skip 'continue'

            if (!check(TokenType::kSemicolon))
                return Result::failure("expected ';' after continue statement", ERRR());
            advance(); // Skip ';'

            result = setLoc(new ContinueStmt(), start);

            break;
        }
        case TokenType::kTry:
        {
            // Parse try-catch statement.

            auto rTry = parseTryCatch();
            CHECK_ERROR(rTry);
            result = rTry.value();

            break;
        }
        case TokenType::kThrow:
        {
            // Parse throw expression.
            advance(); // Skip 'throw'

            auto rTro = parseExpr();
            CHECK_ERROR(rTro);
            if (!check(TokenType::kSemicolon))
            {
                m_diag.error(((check(TokenType::kRBraces) || check(TokenType::kEOF)) ? previous() : current()).location(m_args.file),
                             "expected ';' after throw statement");
                if (!check(TokenType::kRBraces) && !check(TokenType::kEOF))
                {
                    synchronizeStmt();
                }
                result = setLoc(new ThrowStmt(rTro.value()), start);
                break;
            }
            advance(); // Skip ';'
            result = setLoc(new ThrowStmt(rTro.value()), start);

            break;
        }
        case TokenType::kBreak:
        {
            // Parse break statement.
            advance(); // Skip 'break'
            if (!check(TokenType::kSemicolon))
                return Result::failure("expected ';' after break statement", ERRR());
            advance(); // Skip ';'
            result = setLoc(new BreakStmt{}, start);
            break;
        }
        default:
        {
            // fallback : decl parsing
            auto rDecl = parseDecl();
            CHECK_ERROR(rDecl);
            if (rDecl.value() == nullptr)
            {
                auto rTypedDecl = parseTypedVarDecl();
                CHECK_ERROR(rTypedDecl);
                if (rTypedDecl.value() != nullptr)
                {
                    result = setLoc(new DeclStmt(rTypedDecl.value()), start);
                    break;
                }

                // fallback : expression statement

                auto rExpr = parseExpr();
                CHECK_ERROR(rExpr);
                if (rExpr.value() && rExpr.value()->getExprKind() == ExprKind::kError)
                {
                    synchronizeStmt();
                    result = setLoc(new ErrorStmt(), start);
                    break;
                }
                if (!check(TokenType::kSemicolon))
                {
                    const NToken &errTok = (check(TokenType::kRBraces) || check(TokenType::kEOF)) ? previous() : current();
                    m_diag.error(errTok.location(m_args.file), "expected ';' after expression statement");
                    if (!isStmtRecoveryBoundary())
                    {
                        synchronizeStmt();
                    }
                    result = setLoc(new ExprStmt(rExpr.value()), start);
                    break;
                }
                advance(); // Skip ';'
                result = setLoc(new ExprStmt(rExpr.value()), start);
            }
            else
            {
                result = setLoc(new DeclStmt(rDecl.value()), start);
            }
            break;
        }
        }

        return result;
    }

    // Interface parser
    // Syntax like : [...] interface XXX { ... }
    Expected<InterfaceDecl *> NParser::parseInterface()
    {
        const auto start = current();
        if (!check(TokenType::kInterface))
        {
            return nullptr;
        }
        advance(); // Skip 'interface'

        // parse interface's name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected interface name, found ", tokenText(current())), ERRR());
        }
        auto gd = ScopeGuard(new InterfaceDecl(current().value));
        advance(); // Skip name

        if (check(TokenType::kLBraces))
        {
            // Parse interface's body
            advance(); // Skip '{'

            ASTModifier md{};
            Vector<Attribute *> attr{};
            auto syncInterfaceMember = [&]() {
                while (!check(TokenType::kEOF))
                {
                    if (check(TokenType::kIdentifier) || isModifier(current().type) || check(TokenType::kLBracket))
                    {
                        return;
                    }
                    if (check(TokenType::kComma) || check(TokenType::kSemicolon))
                    {
                        advance();
                        return;
                    }
                    if (check(TokenType::kRBraces))
                    {
                        return;
                    }
                    advance();
                }
            };

            do
            {
                if (check(TokenType::kEOF))
                {
                    return Result::failure("expected '}' to close interface body", &m_diag, start, m_args.file);
                }

                if (check(TokenType::kLBracket))
                {
                    // Parse Attribute

                    auto rA = parseAttributes();
                    if (!rA)
                    {
                        syncInterfaceMember();
                        attr = Vector<Attribute *>{};
                        if (check(TokenType::kRBraces))
                        {
                            continue;
                        }
                        continue;
                    }
                    attr = rA.value();
                }
                else if (isModifier(current().type))
                {
                    // Parse modifier if matched.

                    auto r = parseModifier();
                    CHECK_ERROR(r);
                    md = r.value();
                }
                else if (check(TokenType::kIdentifier))
                {
                    // Parse function item

                    const auto fnStart = current();
                    StringView func_name = current().value;
                    advance(); // Skip function's name

                    if (!check(TokenType::kLParen))
                    {
                        m_diag.error(current().location(m_args.file), "expected '(' after interface method name");
                        syncInterfaceMember();
                        md = ASTModifier{};
                        attr = Vector<Attribute *>{};
                        if (check(TokenType::kRBraces))
                        {
                            continue;
                        }
                        continue;
                    }
                    auto rAgs = parseFuncArgs();
                    auto fnc = new FuncDecl();
                    fnc->name = func_name;
                    if (!rAgs)
                    {
                        fnc->modifier = std::move(md);
                        md = ASTModifier{};
                        fnc->attributes = std::move(attr);
                        attr = Vector<Attribute *>{};
                        gd->children.push_back(setLoc(fnc, fnStart));
                        syncInterfaceMember();
                        if (check(TokenType::kRBraces))
                        {
                            continue;
                        }
                        continue;
                    }
                    fnc->args = std::move(rAgs.value());

                    if (check(TokenType::kColon))
                    {
                        advance(); // Skip ':'
                        auto rRet = parseType();
                        if (!rRet)
                        {
                            fnc->returnType = setLoc(new ASTTypeNode("<error>"), current());
                            fnc->modifier = std::move(md);
                            md = ASTModifier{};
                            fnc->attributes = std::move(attr);
                            attr = Vector<Attribute *>{};
                            gd->children.push_back(setLoc(fnc, fnStart));
                            syncInterfaceMember();
                            if (check(TokenType::kRBraces))
                            {
                                continue;
                            }
                            continue;
                        }
                        if (rRet.value() == nullptr)
                        {
                            m_diag.error(current().location(m_args.file), "expected a return type in interface method declaration");
                            fnc->returnType = setLoc(new ASTTypeNode("<error>"), current());
                            fnc->modifier = std::move(md);
                            md = ASTModifier{};
                            fnc->attributes = std::move(attr);
                            attr = Vector<Attribute *>{};
                            gd->children.push_back(setLoc(fnc, fnStart));
                            syncInterfaceMember();
                            if (check(TokenType::kRBraces))
                            {
                                continue;
                            }
                            continue;
                        }
                        fnc->returnType = rRet.value();
                    }
                    else if (isType())
                    {
                        m_diag.error(current().location(m_args.file), "expected ':' before interface method return type");
                        auto rRet = parseType();
                        if (rRet && rRet.value() != nullptr)
                        {
                            fnc->returnType = rRet.value();
                        }
                    }

                    fnc->modifier = std::move(md);
                    md = ASTModifier{};
                    fnc->attributes = std::move(attr);
                    attr = Vector<Attribute *>{};

                    gd->children.push_back(setLoc(fnc, fnStart));

                    if (!check(TokenType::kSemicolon) && !check(TokenType::kComma))
                    {
                        m_diag.error(current().location(m_args.file), "expected ',' or ';' after interface method declaration");
                        syncInterfaceMember();
                        continue;
                    }
                    advance(); // Skip ',' or ';'
                }
                else if (check(TokenType::kRBraces))
                {
                    advance(); // Skip '}'
                    break;
                }
                else
                {
                    m_diag.error(current().location(m_args.file), "invalid declaration in interface body");
                    syncInterfaceMember();
                    if (check(TokenType::kRBraces))
                    {
                        continue;
                    }
                }
            } while (true);
        }
        else if (check(TokenType::kSemicolon))
        {
            // Parse interface defination
            advance(); // Skip ';'
        }
        else
        {
            return Result::failure(msg("unexpected token ", tokenText(current()), " after interface declaration"), ERRR());
        }

        return setLoc(gd.getPtr(), start);
    }

    // Struct parser
    // Syntax like : [...] struct XXX { ... }
    Expected<StructDecl *> NParser::parseStruct()
    {
        const auto start = current();
        if (!check(TokenType::kStruct))
            return nullptr;
        advance(); // Skip 'struct'

        // parse struct's name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected struct name, found ", tokenText(current())), ERRR());
        }
        auto gd = ScopeGuard(new StructDecl(current().value));
        advance(); // Skip name

        if (check(TokenType::kColon))
        {
            // Fallback:
            return Result::failure("struct declarations do not support base types", ERRR());
        }
        else if (check(TokenType::kLBraces))
        {
            // Parse body
            advance(); // Skip '{'

            Attribute attr{};
            ASTModifier md{};

            do
            {
                if (check(TokenType::kVal) || check(TokenType::kVar))
                {
                    // Parse variable

                    auto var = parseVarDecl(false);
                    CHECK_ERROR(var);
                    auto ptr = var.value();
                    if (ptr == nullptr)
                    {
                        return unexpectedToken("struct field declaration");
                    }
                    ptr->modifier = std::move(md);
                    md = ASTModifier{};

                    gd->variables.push_back(ptr);

                    if (!check(TokenType::kComma))
                    {
                        if (check(TokenType::kRBraces))
                            continue;
                        return Result::failure("expected ',' after struct field declaration", ERRR());
                    }
                    advance(); // Skip ','
                }
                else if (check(TokenType::kIdentifier) && expect(TokenType::kColon))
                {
                    auto var = parseColonVarDecl(false);
                    CHECK_ERROR(var);
                    auto ptr = var.value();
                    if (ptr == nullptr)
                    {
                        return unexpectedToken("struct field declaration");
                    }
                    ptr->modifier = std::move(md);
                    md = ASTModifier{};
                    gd->variables.push_back(ptr);

                    if (!check(TokenType::kComma))
                    {
                        if (check(TokenType::kRBraces))
                            continue;
                        return Result::failure("expected ',' after struct field declaration", ERRR());
                    }
                    advance(); // Skip ','
                }
                else if (isModifier(current().type))
                {
                    // Parse modifier if matched.

                    auto rM = parseModifier();
                    CHECK_ERROR(rM);
                    md = rM.value();
                }
                else if (check(TokenType::kRBraces))
                {
                    // Break on '}'
                    advance(); // Skip ';'
                    break;
                }
                else
                {
                    // Fallback:
                    return Result::failure(msg("unexpected token ", tokenText(current()), " in struct body"), ERRR());
                }
            } while (true);
        }
        else
        {
            return Result::failure(msg("unexpected token ", tokenText(current()), " after struct declaration"), ERRR());
        }

        return setLoc(gd.getPtr(), start);
    }

    // If-Stmt parser
    // Syntax like : if (...) {...} else if (...) {...} else {...}
    Expected<IfStmt *> NParser::parseIfStmt(bool onlyIf)
    {
        const auto start = current();
        if (!check(TokenType::kIf))
            return nullptr;
        advance(); // Skip 'if'

        if (!check(TokenType::kLParen))
            return expectedToken("'('", "if condition");

        auto gd = ScopeGuard(new IfStmt());

        advance(); // Skip '('
        // Parse ifExpr
        auto rE = parseExpr();
        CHECK_ERROR(rE);
        gd->ifExpr = rE.value();

        if (!check(TokenType::kRParen))
            return expectedToken("')'", "if condition");
        advance(); // Skip ')'

        if (check(TokenType::kLBraces))
        {
            // Parse if branch's body
            auto rS = parseScope();
            CHECK_ERROR(rS);
            gd->defaultBranch = rS.value();
        }
        else
        {
            // Parse one line if statement.
            auto rB = parseStmt();
            CHECK_ERROR(rB);
            gd->defaultBranch = rB.value();
        }

        if (onlyIf)
            return setLoc(gd.getPtr(), start);

    checkEnd:
        if (check(TokenType::kElse))
        {
            // Parse 'else'
            advance(); // Skip 'else'

            if (check(TokenType::kIf))
            {
                // Parse else if

                auto rN = parseIfStmt(true);
                CHECK_ERROR(rN);
                gd->elseIfBranches.push_back(rN.value());

                goto checkEnd; // Check if 'if' statements end
            }
            else if (check(TokenType::kLBraces))
            {
                // Parse else branch

                auto rS = parseScope();
                CHECK_ERROR(rS);
                gd->elseBranch = rS.value();

                goto checkEnd; // Check if 'if' statements end
            }
            else
            {
                // Parse one-line body
                auto rB = parseStmt();
                CHECK_ERROR(rB);
                gd->elseBranch = rB.value();

                goto checkEnd; // Check if 'if' statements end
            }
        }
        else
        {
            return setLoc(gd.getPtr(), start);
        }
    }

    // Return statement parser
    // Syntax like : return xxx;
    Expected<ReturnStmt *> NParser::parseReturnStmt()
    {
        const auto start = current();
        if (!check(TokenType::kReturn))
            return nullptr;
        advance(); // Skip 'return'

        auto gd = ScopeGuard(new ReturnStmt());
        if (!check(TokenType::kSemicolon))
        {
            auto rE = parseExpr();
            CHECK_ERROR(rE);
            gd->ret = rE.value();
        }

        if (!check(TokenType::kSemicolon))
        {
            m_diag.error(((check(TokenType::kRBraces) || check(TokenType::kEOF)) ? previous() : current()).location(m_args.file),
                         "expected ';' after return statement");
            if (!check(TokenType::kRBraces) && !check(TokenType::kEOF))
            {
                synchronizeStmt();
            }
            return setLoc(gd.getPtr(), start);
        }
        advance(); // Skip ';'
        return setLoc(gd.getPtr(), start);
    }

    // Base class parser
    // Syntax like: : XXX, XXX, ... { [or ;]
    Expected<Vector<ASTTypeNode *>> NParser::parseParents()
    {
        Vector<ASTTypeNode *> baseClasses;

        if (!check(TokenType::kColon))
            return baseClasses;
        advance(); // Skip ':'

        do
        {
            // advance();
            if (check(TokenType::kLBraces) || check(TokenType::kSemicolon))
            {
                break;
            }
            else if (check(TokenType::kComma))
            {
                advance();
                continue;
            }
            else
            {
                auto tp = parseType();
                CHECK_ERROR(tp);
                baseClasses.push_back(tp.value());
            }
        } while (true);

        return baseClasses;
    }

    // Try-catch parser
    // Syntax like: try {...} catch(...) {...} catch {...}
    Expected<TryStmt *> NParser::parseTryCatch()
    {
        const auto start = current();
        // Check 'try {'
        if (!check(TokenType::kTry) || !expect(TokenType::kLBraces))
            return nullptr;
        advance(); // Skip 'try'

        auto rS = parseScope();
        CHECK_ERROR(rS);
        auto gd = ScopeGuard(new TryStmt(rS.value()));
        auto syncCatch = [&]()
        {
            while (!check(TokenType::kEOF))
            {
                if (check(TokenType::kCatch) || check(TokenType::kLBraces) || check(TokenType::kRBraces))
                {
                    return;
                }
                advance();
            }
        };

        if (!check(TokenType::kCatch))
        {
            return Result::failure("try statement requires at least one catch handler", ERRR());
        }

        while (check(TokenType::kCatch))
        {
            // Parse handler

            advance(); // Skip 'catch'
            auto gdH = ScopeGuard(new CatchClause());
            gdH->m_loc = current().location(m_args.file);

            if (check(TokenType::kLParen))
            {
                // Handler with type hint

                // Parse type
                auto rArgs = parseFuncArgs();
                if (!rArgs)
                {
                    syncCatch();
                    if (!check(TokenType::kLBraces))
                    {
                        m_diag.error(current().location(m_args.file), "catch handler declaration is invalid");
                        continue;
                    }
                }
                else
                {
                    auto rD = rArgs.value();
                    if (rD.size() != 1)
                    {
                        m_diag.error(current().location(m_args.file), "catch handler only supports a single exception type");
                    }
                    else
                    {
                        gdH->varName = rD[0]->name;
                        gdH->errorType = rD[0]->type;
                    }
                }
                goto parseBody;
            }
            else if (check(TokenType::kLBraces))
            {
                // Parse body

            parseBody:
                if (!check(TokenType::kLBraces))
                {
                    m_diag.error(current().location(m_args.file), "expected '{' to start catch body");
                    syncCatch();
                    continue;
                }
                auto rSH = parseScope();
                CHECK_ERROR(rSH);
                gdH->handlerBody = rSH.value();
            }
            gd->handlers.push_back(gdH.getPtr());
        }

        return setLoc(gd.getPtr(), start);
    }

    // For loop parser
    // Syntax like: for(...;...;...) {...} or for {...}
    Expected<ForStmt *> NParser::parseForStmt()
    {
        const auto start = current();
        if (!check(TokenType::kFor))
            return nullptr;
        advance(); // Skip 'for'

        auto gd = ScopeGuard(new ForStmt());

        if (check(TokenType::kLParen))
        {
            advance(); // Skip '('
            if (!check(TokenType::kSemicolon))
            {
                auto rVar = parseVarDecl(false);
                CHECK_ERROR(rVar);
                if (rVar.value() != nullptr)
                {
                    gd->declVar = rVar.value();
                }
                else
                {
                    auto rColonVar = parseColonVarDecl(false);
                    CHECK_ERROR(rColonVar);
                    if (rColonVar.value() != nullptr)
                    {
                        gd->declVar = rColonVar.value();
                    }
                    else
                    {
                        auto rTypedVar = parseTypedVarDecl(false);
                        CHECK_ERROR(rTypedVar);
                        if (rTypedVar.value() != nullptr)
                        {
                            gd->declVar = rTypedVar.value();
                        }
                        else
                        {
                            return Result::failure("for-loop initializer must be a variable declaration", ERRR());
                        }
                    }
                }
            }

            if (!check(TokenType::kSemicolon))
                return expectedToken("';'", "for-loop initializer");
            advance(); // Skip ';'

            if (!check(TokenType::kSemicolon))
            {
                auto rExpr = parseExpr();
                CHECK_ERROR(rExpr);
                gd->cond = rExpr.value();
            }

            if (!check(TokenType::kSemicolon))
                return expectedToken("';'", "for-loop condition");
            advance(); // Skip ';'

            if (!check(TokenType::kRParen))
            {
                auto rUpdateExpr = parseExpr();
                CHECK_ERROR(rUpdateExpr);
                gd->update = rUpdateExpr.value();
            }

            if (!check(TokenType::kRParen))
                return expectedToken("')'", "for-loop update");
            advance(); // Skip ')'

            if (check(TokenType::kLBraces))
            {
                goto parseScopeBody;
            }
            else
            {
                // Parse one line for body statement.

                auto rB = parseStmt();
                CHECK_ERROR(rB);
                gd->body = rB.value();
            }
        }
        else if (check(TokenType::kLBraces))
        {
            // Parse scoped statements for for

        parseScopeBody:
            auto rScp = parseScope();
            CHECK_ERROR(rScp);
            gd->body = rScp.value();
        }
        else
        {
            return expectedToken("'(' or '{'", "for statement");
        }

        return setLoc(gd.getPtr(), start);
    }

    // While loop parser
    // Syntax like: while(...) {...} or while {...}
    Expected<WhileStmt *> NParser::parseWhileStmt()
    {
        const auto start = current();
        if (!check(TokenType::kWhile))
            return nullptr;
        advance(); // Skip 'while'
        auto gd = ScopeGuard(new WhileStmt());

        if (check(TokenType::kLParen))
        {
            advance(); // Skip '('

            auto rExpr = parseExpr();
            CHECK_ERROR(rExpr);
            gd->condition = rExpr.value();

            if (!check(TokenType::kRParen))
                return expectedToken("')'", "while condition");
            advance(); // Skip ')'

            if (check(TokenType::kLBraces))
            {
                goto parseBody;
            }
            else
            {
                // Parse one line for body statement.

                auto rB = parseStmt();
                CHECK_ERROR(rB);
                gd->body = rB.value();
            }
        }
        else if (check(TokenType::kLBraces))
        {
            // parse while body.

        parseBody:
            auto rScp = parseScope();
            CHECK_ERROR(rScp);
            gd->body = rScp.value();
        }
        else
        {
            return expectedToken("'(' or '{'", "while statement");
        }

        return setLoc(gd.getPtr(), start);
    }

    // Top level of expression parsing
    Expected<ASTExpr *> NParser::parseExpr(bool skipCommaExpr)
    {
        //        return skipCommaExpr ? parseAssignExpr() : parseCommaExpr();
        return parseAssignExpr();
    }

    // Primary expression parser
    Expected<ASTExpr *> NParser::parsePrimaryExpr()
    {
        const auto start = current();
        if (check(TokenType::kIdentifier))
        {
            // a b c
            auto rId = setLoc(new ASTIdent(current().value), start);
            advance(); // Skip current token
            return rId;
        }
        else if (check(TokenType::kCharLit))
        {
            // 'a'
            auto rCLit = setLoc(new CharLiteralExpr(current().value[0]), start);
            advance(); // Skip current token
            return rCLit;
        }
        else if (check(TokenType::kStringLit))
        {
            // "aaa"
            String value = current().value;

        // deal with "aaa""bbb" -> "aaabbb"
        processStringLit:
            if (expect(TokenType::kStringLit))
            { // TIPS: expect won't cost token!!!
                advance();
                value.append(current().value);
                goto processStringLit;
            }

            auto rSLit = setLoc(new StringLiteralExpr(value), start);
            advance(); // Skip current
            return rSLit;
        }
        else if (check(TokenType::kIntLit) || check(TokenType::kHexLit))
        {
            // 1 2 100 0x00
            auto rNLit = NumberLiteralExpr::parseNumberToken(current().value);
            CHECK_ERROR(rNLit);
            advance();
            return setLoc(rNLit.value(), start);
        }
        else if (check(TokenType::kFloatLit))
        {
            // 8.32
            auto rFLit = NumberLiteralExpr::parseFloatToken(current().value);
            CHECK_ERROR(rFLit);
            advance();
            return setLoc(rFLit.value(), start);
        }
        else if (check(TokenType::kLParen))
        {
            // (...)
            advance(); // Skip '('
            auto rInnerExpr = parseExpr();
            CHECK_ERROR(rInnerExpr);
            if (!check(TokenType::kRParen))
                return expectedToken("')'", "parenthesized expression");
            advance(); // Skip ')'
            return rInnerExpr.value();
        }
        else if (check(TokenType::kTrue) || check(TokenType::kFalse))
        {
            // true false
            auto rBLit = setLoc(new BoolLiteralExpr(current().type == TokenType::kTrue ? (bool)true : (bool)false), start);
            advance(); // Skip current
            return rBLit;
        }
        else if (check(TokenType::kNull))
        {
            auto expr = setLoc(new NullExpr(), start);
            advance(); // Skip current
            return expr;
        }
        else if (check(TokenType::kThis))
        {
            auto expr = setLoc(new ThisExpr(), start);
            advance(); // Skip current
            return expr;
        }
        else if (check(TokenType::kSuper))
        {
            auto expr = setLoc(new SuperExpr(), start);
            advance(); // Skip current
            return expr;
        }
        else if (check(TokenType::kFun))
        {
            // parse lambda function
            auto rLFunc = parseFunc(true);
            CHECK_ERROR(rLFunc);
            return setLoc(new LambdaFuncExpr(rLFunc.value()), start);
        }
        else if (check(TokenType::kCast))
        {
            // cast<i32>(...)
            advance(); // Skip 'cast'
            if (!check(TokenType::kLt))
                return Result::failure("expected type in cast expression", ERRR());
            advance(); // Skip '<'
            auto rCType = parseType();
            CHECK_ERROR(rCType);
            if (!check(TokenType::kGt))
                return Result::failure("expected '>' to close cast type", ERRR());
            advance(); // Skip '>'

            if (!check(TokenType::kLParen))
                return Result::failure("expected cast operand", ERRR());
            advance(); // Skip '('
            auto rCExpr = parseExpr();
            CHECK_ERROR(rCExpr);
            if (!check(TokenType::kRParen))
                return Result::failure("expected ')' to close cast expression", ERRR());
            advance(); // Skip ')'

            return setLoc(new CastExpr(rCExpr.value(), rCType.value()), start);
        }
        else if (check(TokenType::kNew))
        {
            // new Type(args)
            advance(); // Skip 'new'
            auto rNType = parseType();
            CHECK_ERROR(rNType);
            if (rNType.value() == nullptr)
                return expectedToken("a type after 'new'", "new expression");
            Vector<ASTExpr *> args{};
            if (check(TokenType::kLParen))
            {
                auto rArgs = parseFuncCallArgs();
                CHECK_ERROR(rArgs);
                args = std::move(rArgs.value());
            }
            return setLoc(new NewExpr(rNType.value(), std::move(args)), start);
        }
        else if (check(TokenType::kLBracket))
        {
            // [a,b,c]
            advance(); // Skip '['
            Vector<ASTExpr *> elements{};

            if (!check(TokenType::kRBracket))
            {
                while (true)
                {
                    auto elem = parseExpr();
                    CHECK_ERROR(elem);
                    elements.push_back(elem.value());
                    if (check(TokenType::kComma))
                        advance(); // Skip ','
                    else
                        break;
                }
            }

            if (!check(TokenType::kRBracket))
                return Result::failure("expected ']' to close array literal", ERRR());
            advance(); // skip ']'

            return setLoc(new ArrayLiteralExpr(std::move(elements)), start);
        }

        m_diag.error(start.location(m_args.file), "unexpected token " + tokenText(current()) + " in expression");
        synchronizeExpr();
        return makeErrorExpr(start);
    }

    // PostPrefix expression parser
    Expected<ASTExpr *> NParser::parsePostfixExpr()
    {
        auto left = parsePrimaryExpr();
        CHECK_ERROR(left);
        ASTExpr *leftExpr = left.value();
        const auto start = leftExpr->m_loc;

        while (true)
        {
            if (check(TokenType::kLParen))
            {
                // Function invoke expression

                auto args = parseFuncCallArgs();
                CHECK_ERROR(args);
                leftExpr = new CallExpr(leftExpr, args.value());
                leftExpr->m_loc = start;
            }
            else if (check(TokenType::kDot))
            {
                // Member access expression

                if (!expect(TokenType::kIdentifier))
                    return Result::failure(msg("expected member name after '.', found ", tokenText(current())), ERRR());
                advance(); // Skip '.'
                leftExpr = new MemberAccessExpr(leftExpr, current().value);
                leftExpr->m_loc = start;
                advance(); // Skip indentifier
            }
            else if (check(TokenType::kLBracket))
            {
                // Index access expression
                advance(); // Skip '['

                auto index = parseExpr(); // TODO maybe multi-args index?
                CHECK_ERROR(index);
                if (!check(TokenType::kRBracket))
                    return Result::failure("expected ']' to close subscript expression", ERRR());
                advance(); // Skip ']'
                leftExpr = new SubscriptExpr(index.value());
                leftExpr->m_loc = start;
            }
            else if (check(TokenType::kInc))
            {
                // i++
                advance(); // Skip '++'
                auto expr = new PostfixExpr(PostPrefixOp::kInc, leftExpr);
                expr->m_loc = start;
                return expr;
            }
            else if (check(TokenType::kDec))
            {
                // i--
                advance(); // Skip '--'
                auto expr = new PostfixExpr(PostPrefixOp::kDec, leftExpr);
                expr->m_loc = start;
                return expr;
            }
            else
            {
                // TODO support template!!!
                break;
            }
        }

        return leftExpr;
    }

    // Unary expression parser
    Expected<ASTExpr *> NParser::parseUnaryExpr()
    {
        const auto start = current();
        if (check(TokenType::kAdd))
        {
            advance();
            return parseUnaryExpr(); // +(x) -> x
        }

        if (check(TokenType::kLParen))
        {
            psize endIdx = 0;
            if (peek().type == TokenType::kIdentifier && isTypeAt(m_lexer->m_tk_idx + 1, &endIdx) && endIdx < m_lexer->m_tokens.size() && m_lexer->m_tokens[endIdx].type == TokenType::kRParen)
            {
                // (T)a
                advance(); // Skip '('
                auto rType = parseType();
                CHECK_ERROR(rType);
                if (!check(TokenType::kRParen))
                    return Result::failure("expected ')' to close type expression", ERRR());
                advance(); // Skip ')'
                auto rOperand = parseUnaryExpr();
                CHECK_ERROR(rOperand);
                return setLoc(new CastExpr(rOperand.value(), rType.value()), start);
            }
            else
            {
                return parsePostfixExpr();
            }
        }

        UnaryOp op = UnaryOp::kUnknown;
        switch (current().type)
        {
        case TokenType::kInc:
            op = UnaryOp::kPrePlus;
            break;
        case TokenType::kDec:
            op = UnaryOp::kPreMinus;
            break;
        case TokenType::kSub:
            op = UnaryOp::kMinus;
            break;
        case TokenType::kLNot:
            op = UnaryOp::kBang;
            break;
        case TokenType::kBitNot:
            op = UnaryOp::kTilde;
            break;
        case TokenType::kBitAnd:
            op = UnaryOp::kAmp;
            break;
        case TokenType::kMul:
            op = UnaryOp::kStar;
            break;
        default:
            break;
        }

        if (op != UnaryOp::kUnknown)
        {
            advance();
            auto operand = parseUnaryExpr();
            CHECK_ERROR(operand);
            return setLoc(new UnaryExpr(op, operand.value()), start);
        }

        return parsePostfixExpr();
    }

    // Universal binary expression parser
    Expected<ASTExpr *> NParser::parseBinaryExpr(
        Expected<ASTExpr *> (NParser::*subExpr)(),
        const Vector<std::pair<TokenType, BinaryOp>> &ops)
    {
        auto left = (this->*subExpr)();
        CHECK_ERROR(left);
        ASTExpr *leftExpr = left.value();

        while (true)
        {
            BinaryOp op = BinaryOp::kUnknown;
            for (auto &entry : ops)
            {
                if (current().type == entry.first)
                {
                    op = entry.second;
                    break;
                }
            }

            if (op == BinaryOp::kUnknown)
                break;

            advance();
            auto right = (this->*subExpr)();
            CHECK_ERROR(right);

            leftExpr = neo::newObject<BinaryExpr>(op, leftExpr, right.value());
            leftExpr->m_loc = left.value()->m_loc;
        }

        return leftExpr;
    }

    // Multiplicative expression parser
    // Syntax like : a * b / c % d
    Expected<ASTExpr *> NParser::parseMultiplicativeExpr()
    {
        return parseBinaryExpr(&NParser::parseUnaryExpr, {
                                                             {TokenType::kMul, BinaryOp::kMul}, // *
                                                             {TokenType::kDiv, BinaryOp::kDiv}, // /
                                                             {TokenType::kMod, BinaryOp::kMod}, // %
                                                         });
    }

    // Additive expression parser
    // Syntax like : a + b - c
    Expected<ASTExpr *> NParser::parseAdditiveExpr()
    {
        return parseBinaryExpr(&NParser::parseMultiplicativeExpr, {
                                                                      {TokenType::kAdd, BinaryOp::kAdd}, // +
                                                                      {TokenType::kSub, BinaryOp::kSub}, // -
                                                                  });
    }

    // Shift expression parser
    // Syntax like : a << 10 b >> 10
    Expected<ASTExpr *> NParser::parseShiftExpr()
    {
        return parseBinaryExpr(&NParser::parseAdditiveExpr, {
                                                                {TokenType::kShl, BinaryOp::kShl}, // <<
                                                                {TokenType::kShr, BinaryOp::kShr}, // >>
                                                            });
    }

    // Reletional expression parser
    // Syntax like : > < >= <=
    Expected<ASTExpr *> NParser::parseRelationalExpr()
    {
        return parseBinaryExpr(&NParser::parseShiftExpr, {
                                                             {TokenType::kLt, BinaryOp::kLt}, // <
                                                             {TokenType::kGt, BinaryOp::kGt}, // >
                                                             {TokenType::kLe, BinaryOp::kLe}, // >=
                                                             {TokenType::kGe, BinaryOp::kGe}, // <=
                                                         });
    }

    // Equality expression parser
    // Syntax like : != ==
    Expected<ASTExpr *> NParser::parseEqualityExpr()
    {
        return parseBinaryExpr(&NParser::parseRelationalExpr, {
                                                                  {TokenType::kEq, BinaryOp::kEq},   // ==
                                                                  {TokenType::kNeq, BinaryOp::kNeq}, // !=
                                                              });
    }

    // BitAnd expression parser
    // Syntax like : a & b
    Expected<ASTExpr *> NParser::parseBitwiseAndExpr()
    {
        return parseBinaryExpr(&NParser::parseEqualityExpr, {
                                                                {TokenType::kBitAnd, BinaryOp::kBitAnd}, // &
                                                            });
    }

    // BitXor expression parser
    // Syntax like : a ^ b
    Expected<ASTExpr *> NParser::parseBitwiseXorExpr()
    {
        return parseBinaryExpr(&NParser::parseBitwiseAndExpr, {
                                                                  {TokenType::kBitXor, BinaryOp::kBitXor}, // ^
                                                              });
    }

    // BitOr expression parser
    // Syntax like : a | b
    Expected<ASTExpr *> NParser::parseBitwiseOrExpr()
    {
        return parseBinaryExpr(&NParser::parseBitwiseXorExpr, {
                                                                  {TokenType::kBitOr, BinaryOp::kBitOr}, // |
                                                              });
    }

    // LogicalAnd expression parser
    // Syntax like : a && b
    Expected<ASTExpr *> NParser::parseLogicalAndExpr()
    {
        return parseBinaryExpr(&NParser::parseBitwiseOrExpr, {
                                                                 {TokenType::kLAnd, BinaryOp::kLAnd}, // &&
                                                             });
    }

    // LogicalOr expression parser
    // Syntax like : a || b
    Expected<ASTExpr *> NParser::parseLogicalOrExpr()
    {
        return parseBinaryExpr(&NParser::parseLogicalAndExpr, {
                                                                  {TokenType::kLOr, BinaryOp::kLOr}, // ||
                                                              });
    }

    // Conditional expression parser
    // Syntax like : a==b ? c : d
    Expected<ASTExpr *> NParser::parseConditionalExpr()
    {
        auto left = parseLogicalOrExpr();
        CHECK_ERROR(left);

        if (!check(TokenType::kQuestion))
            return left.value();
        advance(); // Skip '?'

        auto trueExpr = parseExpr();
        CHECK_ERROR(trueExpr);

        if (!check(TokenType::kColon))
            return Result::failure("expected ':' in conditional expression", ERRR());
        advance(); // Skip ':'

        auto falseExpr = parseExpr();
        CHECK_ERROR(falseExpr);

        auto expr = neo::newObject<ConditionalExpr>(left.value(), trueExpr.value(), falseExpr.value());
        expr->m_loc = left.value()->m_loc;
        return expr;
    }

    // Assign expression parser
    Expected<ASTExpr *> NParser::parseAssignExpr()
    {
        auto left = parseConditionalExpr();
        CHECK_ERROR(left);

        BinaryOp op = BinaryOp::kUnknown;
        switch (current().type)
        {
        case TokenType::kAssign:
            op = BinaryOp::kAssign;
            break; // =
        case TokenType::kAddAssign:
            op = BinaryOp::kAddAssign;
            break; // +=
        case TokenType::kSubAssign:
            op = BinaryOp::kSubAssign;
            break; // -=
        case TokenType::kMulAssign:
            op = BinaryOp::kMulAssign;
            break; // *=
        case TokenType::kDivAssign:
            op = BinaryOp::kDivAssign;
            break; // /=
        case TokenType::kModAssign:
            op = BinaryOp::kModAssign;
            break; // %=
        case TokenType::kAndAssign:
            op = BinaryOp::kAndAssign;
            break; // &=
        case TokenType::kOrAssign:
            op = BinaryOp::kOrAssign;
            break; // |=
        case TokenType::kXorAssign:
            op = BinaryOp::kXorAssign;
            break; // ^=
        case TokenType::kShlAssign:
            op = BinaryOp::kShlAssign;
            break; // <<=
        case TokenType::kShrAssign:
            op = BinaryOp::kShrAssign;
            break; // >>=
        default:
            break;
        }

        if (op == BinaryOp::kUnknown)
            return left.value();
        advance(); // Skip operator

        auto right = parseAssignExpr();
        CHECK_ERROR(right);

        auto expr = neo::newObject<BinaryExpr>(op, left.value(), right.value());
        expr->m_loc = left.value()->m_loc;
        return expr;
    }

    // TODO Comma expression parser
    Expected<ASTExpr *> NParser::parseCommaExpr()
    {
        //		auto left = parseAssignExpr();
        //		CHECK_ERROR(left);
        //
        //		while(check(TokenType::kComma))
        //		{
        //			advance(); // Skip ','
        //
        //			auto right = parseAssignExpr();
        //			CHECK_ERROR(right);
        //
        //			left = new CommaExpr(left.value(), right.value());
        //		}

        return parseAssignExpr();
    }

    bool NParser::parse()
    {
        auto &output = m_args.output;
        output.clearNodes();

        // parse entry
        auto r = parseRoot();
        if (!r)
        {
            m_diag.printAll();
            return false;
        }

        if (m_diag.hasError())
        {
            m_diag.printAll();
        }

        return true;
    }

    // Check current token is modifier or not
	bool NParser::isModifier(TokenType t)
	{
        return std::find(std::begin(s_modifier), std::end(s_modifier), t) != std::end(s_modifier);
	}

    // Check current is type or expression
    bool NParser::isType()
    {
        return isTypeAt(m_lexer->m_tk_idx);
    }

    void NParser::debugPrint(NDebugOutput &)
    {
    }

#if NE_DEBUG
    bool NParser::debugParse()
    {
        auto &output = m_args.output;
        output.clearNodes();

        // parse entry
        auto r = parseRoot();
        if (!r)
        {
            m_diag.printAll();
            return false;
        }

        if (m_diag.hasError())
        {
            m_diag.printAll();
        }

        return true;
    }
#endif

}


