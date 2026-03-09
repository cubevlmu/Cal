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
#include <vector>

// HINT: all statement parser should advence at last token
//       like ';' at statement's end or ')'/']'/'}' at scope end

// TODO add end line check

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

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
        return Result::failure("unexpected token " + tokenText(current()) + " while parsing " + context, ERRR());
    }

    Result NParser::expectedToken(const String &expected, const String &context)
    {
        return Result::failure("expected " + expected + " while parsing " + context + ", but found " + tokenText(current()), ERRR());
    }

    bool NParser::isTypeAt(psize idx, psize *endIdx) const
    {
        const auto &tokens = m_lexer->m_tokens;
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
                auto p_import_Ret = parseImport();
                CHECK_ERROR(p_import_Ret);
                output.Nodes.push_back(p_import_Ret.value());
            }
            else if (check(TokenType::kEOF))
            {
                advance();
                break;
            }
            else
            {
                auto r = parseDecl();
                CHECK_ERROR(r);
                if (r.value() == nullptr)
                {
                    return unexpectedToken("top-level declaration");
                }
                output.Nodes.push_back(r.value());
            }
        } while (true);

        return Result::success();
    }

    // import statement parser
    // suppoting module string lit like "aaa.bbb.ccc"
    // TESTED
    Expected<ImportStmt *> NParser::parseImport()
    {
        auto start = current();
        if (!check(TokenType::kImport))
        {
            return unexpectedToken("import declaration");
        }
        neo::NString moduleName{};

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

        return setLoc(neo::newObject<ImportStmt>(moduleName), start);
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
        auto children = ScopeGuard(new TopLevelDecls());

        if (check(TokenType::kSemicolon))
        {
            // top level module decl
            // trigger decl parsing logic and make those decls as module's children
            advance(); // Skip ';'

            do
            {
                if (check(TokenType::kEOF))
                {
                    advance(); // Skip 'EOF' ///???????
                    break;
                }
                else
                {
                    auto r = parseDecl();
                    CHECK_ERROR(r);
                    children->decls.push_back(r.value());
                }
            } while (true);

            gd->children = children.getPtr();
        }
        else if (check(TokenType::kLBraces))
        {
            // scope-based module decl
            // trigger scope decl parsing logic and make those decls as module's children
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
        if (!check(TokenType::kFun))
        {
            return unexpectedToken("function declaration");
        }
        if (!isLambda && !expect(TokenType::kIdentifier))
        {
            return expectedToken("a function name", "function declaration");
        }
        advance();
        String name{};

        if (!isLambda)
        {
            // function name parsing logic
            name = current().value;
            advance(); // Skip name

            auto rGeneric = parseGenericSuffix(name);
            CHECK_ERROR(rGeneric);
        }

        if (!check(TokenType::kLParen))
        {
            return expectedToken("'('", "function parameter list");
        }

        // function argument parsing
        auto args = parseFuncArgs();
        CHECK_ERROR(args);
        for (auto *arg : args.value())
        {
            if (arg && arg->m_loc.file == nullptr)
            {
                arg->m_loc = start.location(m_args.file);
            }
        }

        // function return type parsing
        ASTTypeNode *returnType = nullptr;
        if (check(TokenType::kIdentifier))
        {
            auto t = parseType();
            CHECK_ERROR(t);
            returnType = t.value();
        }

        // check scope-based decl or interface-based decl
        if (check(TokenType::kSemicolon))
        {
            // end with ';' just return
            advance();
            return setLoc(new FuncDecl(name, returnType, args.value(), nullptr), start);
        }
        else if (check(TokenType::kLBraces))
        {
            // end with '{'

            auto r = parseScope();
            CHECK_ERROR(r);
            return setLoc(new FuncDecl(name, returnType, args.value(), r.value()), start);
        }
        else
        {
            return expectedToken("';' or '{'", "function declaration");
        }
    }

    Expected<void> NParser::parseGenericSuffix(String &out)
    {
        if (!check(TokenType::kLt))
        {
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

        return Result::success();
    }

    // common type paring function
    // dealing normal type & array type & pointer type
    // TESTED
    Expected<ASTTypeNode *> NParser::parseType()
    {
        const auto start = current();
        if (!check(TokenType::kIdentifier))
        {
            return nullptr;
        }

        // get full type string including module and type
        String typeStr;
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

        auto rGeneric = parseGenericSuffix(typeStr);
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
                        return Result::failure("Unexpected token found in array type brackets -> ]' or not closed.", ERRR());
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
            if (std::find(&s_modifier[0], &s_modifier[7], current().type) == &s_modifier[7])
            {
                break;
            }

            switch (current().type)
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

        if (!check(TokenType::kLParen))
            return args;
        advance(); // Skip '('
        do
        {
            if (check(TokenType::kLBracket))
            {
                // parse attributes

                auto att = parseAttributes();
                CHECK_ERROR(att);
                attrs = att.value();
            }
            if (check(TokenType::kIdentifier) && expect(TokenType::kColon))
            {
                const auto argStart = current();
                auto r = parseModifier();
                CHECK_ERROR(r);
                auto md = r.value();

                String &arg_name = current().value;
                advance();
                if (!expect(TokenType::kIdentifier))
                {
                    advance();
                    return Result::failure(msg("expect type identifier for function argument, but receive '", current().value, "'"), ERRR());
                }
                advance();
                auto t = parseType();
                CHECK_ERROR(t);

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
                    return Result::failure("unexpected expression after function argument declareation \"xxx : xxx [xxx] -> ...\"", ERRR());
                }
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
                auto r = parseStmt();
                CHECK_ERROR(r);
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
                    return Result::failure(msg("expect ']' to close attribute attach but got '", current().value, "'"), ERRR());
                }
                advance(); // Skip ']'
                attrs.push_back(g.getPtr());
            }
            else
            {
                CLEARUP(attrs);
                return Result::failure(msg("unexpect token '", current().value, "' after attribute attach's name"), ERRR());
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

        if (check(TokenType::kLBracket))
        {
            // parse attribute

            auto p_attribute_Ret = parseAttributes();
            CHECK_ERROR(p_attribute_Ret);
            attrs = p_attribute_Ret.value();
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
        if (!check(TokenType::kClass))
        {
            return nullptr;
        }
        advance();

        // class name parsing
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure(msg("expected identifier for class name but got : '", current().typeString(), "'"), ERRR());
        }
        String name = current().value;

        // super classes parsing
        auto gd = ScopeGuard<ClassDecl>(new ClassDecl(name, {}));

        // pre-def for body parsing
        Vector<Attribute *> attrs{};
        ASTModifier md{};

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
                return Result::failure(msg("unexpect token '", current().typeString(), "' for class declare"), ERRR());
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
            return Result::failure(msg("unexpect token '", current().typeString(), "' for class declare"), ERRR());
        }

    parseBody:
        // class body parsing

        do
        {
            advance();

            if (check(TokenType::kLBracket))
            {
                // attributes parsing

                auto rq = parseAttributes();
                CHECK_ERROR(rq);
                attrs = rq.value();
            }
            else if (check(TokenType::kIdentifier))
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
                auto r = parseFunc();
                CHECK_ERROR(r);
                APPLY_MODIFIER_RAW(r, md);
                APPLY_ATTRIBUTES(r, attrs);
                if (type == 1)
                {
                    gd->ctors.push_back(r.value());
                }
                else if (type == 2)
                {
                    if (gd->dtors != nullptr)
                        return Result::failure("redefined destructor.", ERRR());
                    gd->dtors = r.value();
                }
                else
                {
                    gd->functions.push_back(r.value());
                }
            }
            else if (check(TokenType::kField))
            {
                // class field parsing

                auto r = parseField();
                CHECK_ERROR(r);
                APPLY_MODIFIER_RAW(r, md);
                APPLY_ATTRIBUTES(r, attrs);
                gd->fields.push_back(r.value());
            }
            else if (check(TokenType::kRBraces))
            {
                // end scope parsed

                break;
            }
            else
            {
                auto dr = parseDecl();
                CHECK_ERROR(dr);
                APPLY_MODIFIER_RAW(dr, md);
                APPLY_ATTRIBUTES(dr, attrs);
                auto *dk = dr.value();

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
                    return Result::failure("unexpected type declared in class body", ERRR());
                }
            }
        } while (true);

    end:
        advance();
        return gd.getPtr();
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
            return Result::failure("unexpected token found after var/val : var xxx <--", ERRR());
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
            return Result::failure("typed declaration expects a variable name after type.", ERRR());
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
                return Result::failure("typed declaration should end with ';'.", ERRR());
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
            return Result::failure("expected ':' after loop variable name.", ERRR());
        }
        advance();

        auto rType = parseType();
        CHECK_ERROR(rType);
        if (rType.value() == nullptr)
        {
            return Result::failure("expected type after ':' in variable declaration.", ERRR());
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
                return Result::failure("variable declaration should end with ';'.", ERRR());
            }
            advance();
        }

        return setLoc(gd.getPtr(), start);
    }

    // enum parser
    // syntax like 'enum XXX : XXX { ... }'
    Expected<EnumDecl *> NParser::parseEnum()
    {
        if (!check(TokenType::kEnum))
        {
            return nullptr;
        }
        advance();

        // parse enum name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure("unexpected token after enum token : enum xxx <--", ERRR());
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
                return gd.getPtr();
            }
            else
            {
                return Result::failure("unexpected token after enum head declare : enum xxx : xxx ... <--", ERRR());
            }
        }
        else if (check(TokenType::kLBraces))
        {
            // parse enum body

        parseBody:

            do
            {
                advance();
                if (check(TokenType::kIdentifier))
                {
                    StringView itemName = current().value;

                    advance();
                    if (check(TokenType::kAssign))
                    {
                        // parse enum assignment

                        advance();
                        auto eas = parseExpr();
                        CHECK_ERROR(eas);

                        // check next
                        if (check(TokenType::kComma))
                        {
                            advance();
                        }
                        gd->children.push_back(new VarDecl(itemName, nullptr, eas.value()));
                    }
                    else if (check(TokenType::kComma))
                    {
                        gd->children.push_back(new VarDecl(itemName, nullptr));
                        continue;
                    }
                    else
                    {
                        return Result::failure("Invalied expression in enum body", ERRR());
                    }
                }
                else if (check(TokenType::kRBraces))
                {
                    // end scope

                    break;
                }
                else if (check(TokenType::kComma))
                {
                    advance();
                }
            } while (true);
        }
        else
        {
            return Result::failure("unexpected token after enum head declare : enum xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // field parser
    // syntax like 'field xxx : xxx {XXX,XXX} = xxx;'
    Expected<FieldDecl *> NParser::parseField()
    {
        if (!check(TokenType::kField))
        {
            return nullptr;
        }
        advance();

        if (!check(TokenType::kIdentifier))
        {
            return Result::failure("unexpected token after field keyword : field xxx <--", ERRR());
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
                return Result::failure("unexpected token after field's type hint : field xxx : xxx ... <--", ERRR());
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
                    return Result::failure("unexpected token after field's read function : field xxx {XXX ... <--", ERRR());
                }
                advance();
            }
            else if (check(TokenType::kComma))
            {
                advance();
            }
            else
            {
                return Result::failure("unexpected token in field's body : field xxx {... <--", ERRR());
            }

            // check write function name
            if (check(TokenType::kIdentifier))
            {
                StringView funcName = current().value;
                gd->setFuncName = funcName;
                advance();

                if (!check(TokenType::kRBraces))
                {
                    return Result::failure("unexpected token after field's write function : field xxx {XXX,XXX... <--", ERRR());
                }
                advance();
            }
            else if (check(TokenType::kRBraces))
            {
                advance();
            }
            else
            {
                return Result::failure("unexpected token in field's body : field xxx {XXX,... <--", ERRR());
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
            else
            {
            errorBc:
                return Result::failure("unexpected token after field declare expression : field xxx {XXX,XXX} = XXX... <--", ERRR());
            }
        }
        else
        {
            return Result::failure("unexpected token after field's name : field XXX ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // Statement parser
    // syntax ...
    Expected<ASTStmt *> NParser::parseStmt()
    {
        const auto start = current();
        ASTStmt *result = nullptr;

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
                return Result::failure("need ; after continue to close the statement.", ERRR());
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
                return Result::failure("throw statement should end with semicolon ';'.", ERRR());
            advance(); // Skip ';'
            result = setLoc(new ThrowStmt(rTro.value()), start);

            break;
        }
        case TokenType::kBreak:
        {
            // Parse break statement.
            advance(); // Skip 'break'
            if (!check(TokenType::kSemicolon))
                return Result::failure("need ; after break to close the statement.", ERRR());
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
                if (!check(TokenType::kSemicolon))
                {
                    return Result::failure("expression line should end with semi-colon ';' ", ERRR());
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
        if (!check(TokenType::kInterface))
        {
            return nullptr;
        }
        advance(); // Skip 'interface'

        // parse interface's name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure("unexpected token after interface keyword : interface ... <--", ERRR());
        }
        auto gd = ScopeGuard(new InterfaceDecl(current().value));
        advance(); // Skip name

        if (check(TokenType::kLBraces))
        {
            // Parse interface's body
            advance(); // Skip '{'

            ASTModifier md{};
            Vector<Attribute *> attr{};

            do
            {
                if (check(TokenType::kLBracket))
                {
                    // Parse Attribute

                    auto rA = parseAttributes();
                    CHECK_ERROR(rA);
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

                    StringView func_name = current().value;
                    advance(); // Skip function's name

                    if (!check(TokenType::kLParen))
                    {
                        return Result::failure("unexpected token after function's name", ERRR());
                    }
                    auto rAgs = parseFuncArgs();
                    CHECK_ERROR(rAgs);
                    auto fnc = new FuncDecl();
                    fnc->name = func_name;
                    fnc->modifier = std::move(md);
                    md = ASTModifier{};
                    fnc->attributes = std::move(attr);
                    attr = Vector<Attribute *>{};

                    gd->children.push_back(fnc);

                    if (!check(TokenType::kSemicolon))
                    {
                        return Result::failure("function declare was not closed", ERRR());
                    }
                    advance(); // Skip ';'
                }
                else
                {
                    return Result::failure("unexpected identifier in interface body", ERRR());
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
            return Result::failure("unexpected token after interface's name : interface xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // Struct parser
    // Syntax like : [...] struct XXX { ... }
    Expected<StructDecl *> NParser::parseStruct()
    {
        if (!check(TokenType::kStruct))
            return nullptr;
        advance(); // Skip 'struct'

        // parse struct's name
        if (!check(TokenType::kIdentifier))
        {
            return Result::failure("unexpected token after struct keyword : struct ... <--", ERRR());
        }
        auto gd = ScopeGuard(new StructDecl(current().value));
        advance(); // Skip name

        if (check(TokenType::kColon))
        {
            // Fallback:
            return Result::failure("Struct not support with parent classes!", ERRR());
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

                    auto var = parseVarDecl();
                    CHECK_ERROR(var);
                    auto ptr = var.value();
                    ptr->modifier = std::move(md);
                    md = ASTModifier{};

                    gd->variables.push_back(ptr);

                    if (!check(TokenType::kComma))
                    {
                        if (expect(TokenType::kRBraces))
                            continue;
                        return Result::failure("Need comma to split variable declaration.", ERRR());
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
                    return Result::failure("Unexpected token found in struct's body.", ERRR());
                }
            } while (true);
        }
        else
        {
            return Result::failure("Unexpected token after struct's name", ERRR());
        }

        return gd.getPtr();
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
            return Result::failure("return statement if not closed by semicolon, need ';' after return xxx <--", ERRR());
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

        do
        {
            advance();
            if (check(TokenType::kLBraces) || check(TokenType::kSemicolon))
            {
                break;
            }
            else if (check(TokenType::kComma))
            {
                advance();
                break;
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

    parseAgain:
        if (check(TokenType::kCatch))
        {
            // Parse handler

            advance(); // Skip 'catch'
            auto gdH = ScopeGuard(new CatchStmt());
            gdH->m_loc = current().location(m_args.file);
            gdH->errorType = nullptr;

            if (check(TokenType::kLParen))
            {
                // Handler with type hint

                // Parse type
                auto rArgs = parseFuncArgs();
                CHECK_ERROR(rArgs);
                auto rD = rArgs.value();
                if (rD.size() != 1)
                {
                    return Result::failure("Catch handler only avaliable for single exception type!", ERRR());
                }
                gdH->errorType = rD[0];
                goto parseBody;
            }
            else if (check(TokenType::kLBraces))
            {
                // Parse body

            parseBody:
                if (!check(TokenType::kLBraces))
                    return Result::failure("Catch body not found!", ERRR());
                auto rSH = parseScope();
                CHECK_ERROR(rSH);
                gdH->handlerBody = rSH.value();
            }
            gd->handlers.push_back(gdH.getPtr());

            // Check if any other handler exist.
            if (check(TokenType::kCatch))
                goto parseAgain;
            else
                return setLoc(gd.getPtr(), start);
        }
        else
        {
            return Result::failure("Try block without any catch handler", ERRR());
        }
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
                            return Result::failure("for statement initializer should be a variable declaration.", ERRR());
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
                gd->forBody = rB.value();
            }
        }
        else if (check(TokenType::kLBraces))
        {
            // Parse scoped statements for for

        parseScopeBody:
            auto rScp = parseScope();
            CHECK_ERROR(rScp);
            gd->forBody = rScp.value();
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
                return Result::failure("No type hint in cast expression -> <...", ERRR());
            advance(); // Skip '<'
            auto rCType = parseType();
            CHECK_ERROR(rCType);
            if (!check(TokenType::kGt))
                return Result::failure("Type hint is not closed in cast expression -> ...>", ERRR());
            advance(); // Skip '>'

            if (!check(TokenType::kLParen))
                return Result::failure("No cast body -> (...", ERRR());
            advance(); // Skip '('
            auto rCExpr = parseExpr();
            CHECK_ERROR(rCExpr);
            if (!check(TokenType::kRParen))
                return Result::failure("Cast body not closed -> ...)", ERRR());
            advance(); // Skip ')'

            return setLoc(new CastExpr(rCExpr.value(), rCType.value()), start);
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
                return Result::failure("array literal not closed -> ']'", ERRR());
            advance(); // skip ']'

            return setLoc(new ArrayLiteralExpr(std::move(elements)), start);
        }

        return Result::failure("Unexpected token in primary expression: " + current().toString(), ERRR());
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
                    return Result::failure("exptected identifier but got unexpected token after member access dot", ERRR());
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
                    return Result::failure("subscript expression not closed -> ']'", ERRR());
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
            else if (check(TokenType::kNew))
            {
                // new xxx(...);
                advance(); // Skip 'new'
                auto rNType = parseType();
                CHECK_ERROR(rNType);
                if (!check(TokenType::kLParen))
                    return Result::failure("new instance should provide a argument body.");
                auto rNArgs = parseFuncCallArgs();
                CHECK_ERROR(rNArgs);
                return setLoc(new NewExpr(rNType.value(), rNArgs.value()), current());
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
                    return Result::failure("type body is not closed -> ...)", ERRR());
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
            return Result::failure("expected : in conditional expression but got none.", ERRR());
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

        // dump ast logic

        return true;
    }

    // Check current token is modifier or not
    bool NParser::isModifier(TokenType t)
    {
        return std::find(&s_modifier[0], &s_modifier[7], t) != &s_modifier[7];
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

        // dump ast logic

        return true;
    }
#endif

}
