// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Parser.hpp"

#include "neo/ast/Type.hpp"
#include "neo/ast/Decl.hpp"
#include "neo/diagnose/Diagnostic.hpp"
#include "neo/compiler/Lexer.hpp"
#include "neo/compiler/Tokens.hpp"
#include "ParsedFile.hpp"
#include "neo/ast/Stmts.hpp"

#include <nbase/utils/StringUtils.hpp>
#include <sstream>
#include <vector>

//HINT: all statement parser should advence at last token
//      like ';' at statement's end or ')'/']'/'}' at scope end

//TODO add end line check

namespace neo {

    template <typename T>
    struct ScopeGuard
    {
        ScopeGuard(T* ptr) : m_ptr{ ptr }, m_used{ false } {}
        ~ScopeGuard() {
            if (m_used) return;
            delete m_ptr;
        }

        T* getPtr() {
            m_used = true;
            return m_ptr;
        }

        T* operator->() {
            return m_ptr;
        }

    private:
        bool m_used;
        T* m_ptr;
    };


    TokenType NParser::s_modifier[] = {
        TokenType::kInline,
        TokenType::kStatic,
        TokenType::kConst,
        TokenType::kExport,
        TokenType::kInternal,
        TokenType::kProtected,
        TokenType::kPrivate,
    };

#define CHECK_MODIFIER(ITEM, ITEM_NAME) if (ITEM) { return Result::failure("duplicated modifier "#ITEM_NAME); }
#define ERRR() &m_diag, current(), m_args.file
#define CLEARUP(V) \
do { \
    for (auto* ptr : V) { \
        delete ptr; \
    } \
    V.clear();\
} while(false) \

    NParser::NParser(NParserArgs args)
        : m_args{ args }
        , m_lexer{ args.lexer }
    {
    }

    NParser::~NParser()
    {
        m_diag.clear();
    }

    NToken& NParser::advance() {
        return m_lexer->nextToken();
    }
    NToken& NParser::current() {
        return m_lexer->current();
    }
    NToken& NParser::peek() {
        return m_lexer->peekNext();
    }
    NToken& NParser::peekPrevious() {
        return m_lexer->peekPrevious();
    }
    NToken& NParser::previous() {
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

    // top-statement parser
    // import parser + decls parser
    Expected<void> NParser::parseRoot()
    {
        auto& output = m_args.output;

        do {
            if (check(TokenType::kImport)) {
                auto p_import_Ret = parseImport();
                CHECK_ERROR(p_import_Ret);
                output.Nodes.push_back(p_import_Ret.value());
            }
            else if (check(TokenType::kEOF)) {
                advance();
                break;
            }
            else {
                auto r = parseDecl();
                CHECK_ERROR(r);
                output.Nodes.push_back(r.value());
            }
        } while (true);

        return Result::success();
    }



    // import statement parser
    // suppoting module string lit like "aaa.bbb.ccc"
    // TESTED
    Expected<ImportStmt*> NParser::parseImport()
    {
        if (!check(TokenType::kImport)) {
            return Result::failure(msg("unexpected token '", current().typeString(), "' for import expression"), &m_diag, current(), m_args.file);
        }
        std::string moduleName;

        // parse import string lit, module names concat with dot
        do {
            advance();
            if (check(TokenType::kIdentifier)) {
                moduleName.append(current().value);
            }
            else if (check(TokenType::kDot)) {
                moduleName.append(".");
            }
            else if (check(TokenType::kSemicolon)) {
                advance();
                break;
            }
            else {
                // invalid types...
                return Result::failure(msg("unexpected token '", current().typeString(), "' for module name"), ERRR());
            }
        } while (true);

        return new ImportStmt(moduleName);
    }

    // module declare parser
    // support syntax like "module aaa;" or "module bbb {...}"
    // TESTED
    Expected<ModuleDecl*> NParser::parseModule()
    {
        if (!check(TokenType::kModule)) {
            return nullptr;
        }
        std::string module{};

        // parse module name decl
        // support syntax like 'aaa' or 'aaa.bbb'
        // end at ';' or '{'
        do {
            advance();
            if (check(TokenType::kIdentifier)) {
                module.append(current().value);
            }
            else if (check(TokenType::kDot)) {
                module.append(".");
            }
            else if (check(TokenType::kSemicolon) || check(TokenType::kLBraces)) {
                break;
            }
            else {
                return Result::failure(msg("unexpected token '", current().typeString(), "' for module declare"), ERRR());
            }
        } while (true);
        auto gd = ScopeGuard(new ModuleDecl(module));

        if (check(TokenType::kSemicolon)) {
            // top level module decl
            // trigger decl parsing logic and make those decls as module's children

            gd->children = new TopLevelDecls();
            do {
                advance();
                if (check(TokenType::kEOF)) {
                    advance();
                    break;
                } else {
                    auto r = parseDecl();
                    CHECK_ERROR(r);
                    gd->children->decls.push_back(r.value());
                }
            } while(true);
        }
        else if (check(TokenType::kLBraces)) {
            // scope-based module decl
            // trigger scope decl parsing logic and make those decls as module's children
            advance(); // eat '{'

            do {
                if (check(TokenType::kRBraces)) {
                    advance(); // eat '}'
                    break;
                } else {
                    auto r = parseDecl();
                    CHECK_ERROR(r);
                    gd->children->decls.push_back(r.value());
                }
            } while(true);
        }
        else {
            // invalid syntax
            return Result::failure(msg("expect ';' or '{' behind module declare statement, but found '", current().typeString(), "'"), ERRR());
        }

        return gd.getPtr();
    }

    // function declaration parser
    // syntax like xxx fun xxx(...) xxx {...}
    // TESTED
    Expected<FuncDecl*> NParser::parseFunc()
    {
        if (!check(TokenType::kFun) && expect(TokenType::kIdentifier)) {
            return Result::failure(msg("unexpected token for function declare : ", current().value, " ", peek().value), ERRR());
        }
        advance();

        // function name parsing logic
        std::string name = current().value;

        if (!expect(TokenType::kLParen)) {
            return Result::failure(msg("function declare expect '(' for function arguments but got '", peek().value, "'"), ERRR());
        }
        advance();

        // function argument parsing
        auto args = parseFuncArgs();
        CHECK_ERROR(args);

        // function return type parsing
        ASTTypeNode* returnType = nullptr;
        if (check(TokenType::kIdentifier)) {
            auto t = parseType();
            CHECK_ERROR(t);
            returnType = t.value();
        }

        // check scope-based decl or interface-based decl
        if (check(TokenType::kSemicolon)) {
            // end with ';' just return

            return new FuncDecl(name, returnType, args.value(), nullptr);
        }
        else if (check(TokenType::kLBraces)) {
            // end with '{'

            auto r = parseScope();
            CHECK_ERROR(r);
            return new FuncDecl(name, returnType, args.value(), r.value());
        } else {
            return Result::failure("unexpected token after function head", ERRR());
        }
    }

    // common type paring function
    // dealing normal type & array type & pointer type
    // TESTED
    Expected<ASTTypeNode*> NParser::parseType()
    {
        if (!check(TokenType::kIdentifier)) {
            return nullptr;
        }

        // get full type string including module and type
        std::string typeStr;
        typeStr.append(current().value);
        advance();

        while (check(TokenType::kDot)) {
            advance(); // eat dot

            if (!check(TokenType::kIdentifier)) {
                return Result::failure("expected identifier after '.' in type name", ERRR());
            }

            typeStr.append(".");
            typeStr.append(current().value);
            advance();
        }

        if (check(TokenType::kLBracket)) {
            // parse array type's bracket and check array dimenssion

            advance(); // eat left bracket '['
            ScopeGuard<ASTArrayType> gd {new ASTArrayType(std::move(typeStr), false, {})};
            do {
                if (check(TokenType::kIntLit)) {
                    gd->size.push_back(std::stoi(current().value));
                    advance();
                    continue;
                } else if (check(TokenType::kComma)) {
                    advance();
                    continue;
                } else if (check(TokenType::kRBracket)) {
                    advance();
                    break;
                } else {
                    return Result::failure("Unexpected token found in array type brackets -> ]' or not closed.", ERRR());
                }
            } while(true);
            gd->dimenssion = (i32)gd->size.size();
            if (gd->size.empty()) {
                gd->isReceiver = true;
            }
            return gd.getPtr();

        } else if (check(TokenType::kMul)) {
            // parse pointer type

            advance();
            return new ASTPointerType(std::move(typeStr));
        } else {
            // normal type just return

            return new ASTTypeNode(std::move(typeStr));
        }
    }

    // modifier parser
    // support all modifier register in s_modifier
    // TESTED
    Expected<ASTModifier> NParser::parseModifier()
    {
        ASTModifier mf{};

        do {
            if (std::find(&s_modifier[0], &s_modifier[7], current().type) == &s_modifier[7]) {
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
                default:break;
            }

            advance();
        } while (true);

        return mf;
    }



    // fuction calling expression's argument list parser
    // syntax like xxx(aa,bb,cc,...)
    Expected<std::vector<ASTExpr*>> NParser::parseFuncCallArgs()
    {
        std::vector<ASTExpr*> args{};
        if (!check(TokenType::kLParen)) {
            return args;
        }

        do {
            advance();
            if (check(TokenType::kComma)) {
                advance();
            } else if (check(TokenType::kRParen)) {
                advance();
                break;
            } else {
                auto r = parseExpr();
                CHECK_ERROR(r);
                args.push_back(r.value());
            }
        } while (true);

        return args;
    }

    // function's argument parser
    // syntax like (xx : xx, xx : xx = xx, ...)
    Expected<std::vector<VarDecl*>> NParser::parseFuncArgs() {
        // function argument parsing logic
        std::vector<VarDecl*> args{};
        std::vector<Attribute*> attrs{};

        advance(); // eat left paren '('
        do {
            if (check(TokenType::kLBracket)) {
                // parse attributes

                auto att = parseAttributes();
                CHECK_ERROR(att);
                attrs = att.value();
            }
            if (check(TokenType::kIdentifier) && expect(TokenType::kColon)) {
                auto r = parseModifier();
                CHECK_ERROR(r);
                auto md = r.value();

                std::string& arg_name = current().value;
                advance();
                if (!expect(TokenType::kIdentifier)) {
                    advance();
                    return Result::failure(msg("expect type identifier for function argument, but receive '", current().value, "'"), ERRR());
                }
                advance();
                auto t = parseType();
                CHECK_ERROR(t);

                if (check(TokenType::kEq)) {
                    advance();
                    auto epr = parseExpr();
                    CHECK_ERROR(epr);
                    args.push_back(new VarDecl(arg_name, t.value(), epr.value()));
                    args.back()->attributes = std::move(attrs);
                    args.back()->modifier = std::move(md);
                    attrs = std::vector<Attribute*>{};

                    continue;
                } else if (check(TokenType::kComma) || check(TokenType::kRParen)) {
                    args.push_back(new VarDecl(arg_name, t.value()));
                    args.back()->attributes = std::move(attrs);
                    args.back()->modifier = std::move(md);
                    attrs = std::vector<Attribute*>{};

                    // only break when meet ')'
                    if (check(TokenType::kRParen)) {
                        advance();
                        break;
                    }
                    advance();
                    continue;
                } else {
                    return Result::failure("unexpected expression after function argument declareation \"xxx : xxx [xxx] -> ...\"", ERRR());
                }
            } else if (check(TokenType::kComma)) {
                // skip comma

                advance();
                continue;
            } else if (check(TokenType::kRParen)) {
                // end loop when matched ')'

                advance();
                break;
            }
        } while (true);

        return args;
    }

    // scope statments parser
    // syntax like { ... }
    Expected<CompoundStmt*> NParser::parseScope() {
        if (!check(TokenType::kLBraces)) {
            return nullptr;
        }
        advance(); // eat '{'
        auto gd = ScopeGuard(new CompoundStmt());

        do {
            if (check(TokenType::kRBraces)) {
                advance(); // eat '}'
                break;
            } else {
                auto r = parseStmt();
                CHECK_ERROR(r);
                gd->statements.push_back(r.value());
            }
        } while(true);

        return gd.getPtr();
    }

    // parse attributes on decls
    // syntax like [xxx(...)] or [xxx]
    Expected<std::vector<Attribute*>> NParser::parseAttributes()
    {
        std::vector<Attribute*> attrs{};

        do {
            // check '[xxx'
            if (!check(TokenType::kLBracket) && expect(TokenType::kIdentifier)) {
                return attrs;
            }
            advance();

            // get attribute's string-lit
            std::string name = current().value;
            ScopeGuard g{ new Attribute {} };
            g->name = name;

            // check '[xxx(' <- and parse args
            if (!expect(TokenType::kLParen) && !expect(TokenType::kRBracket)) {
                return attrs;
            }
            advance();

            if (check(TokenType::kLParen)) {
                // parse attribute's arguments

                auto r = parseFuncCallArgs();
                CHECK_ERROR(r);
                g->arguments.swap(r.value());
                if (!expect(TokenType::kRBracket)) {
                    CLEARUP(attrs);
                    return Result::failure(msg("expect ']' to close attribute attach but got '", current().value, "'"), ERRR());
                }
                attrs.push_back(g.getPtr());
            }
            else if (check(TokenType::kRBracket)) {
                attrs.push_back(g.getPtr());
            }
            else {
                CLEARUP(attrs);
                return Result::failure(msg("unexpect token '", current().value, "' after attribute attach's name"), ERRR());
            }
        } while (expect(TokenType::kLBracket));

        return attrs;
    }

    // declaration parser
    // for function decl/class decl/struct decl/module decl...
    Expected<ASTDecl*> NParser::parseDecl()
    {
        auto md = parseModifier();
        CHECK_ERROR(md);
        std::vector<Attribute*> attrs {};

        if (check(TokenType::kLBracket)) {
            // parse attribute

            auto p_attribute_Ret = parseAttributes();
            CHECK_ERROR(p_attribute_Ret);
            attrs = p_attribute_Ret.value();
        }
        if (check(TokenType::kModule)) {
            // module decl parsing logic

            auto p_module_Ret = parseModule();
            CHECK_ERROR(p_module_Ret);
            APPLY_MODIFIER(p_module_Ret, md);
            APPLY_ATTRIBUTES(p_module_Ret, attrs);
            return p_module_Ret.value();
        }
        else if (check(TokenType::kFun)) {
            // function decl parsing logic

            auto p_func_Ret = parseFunc();
            CHECK_ERROR(p_func_Ret);
            APPLY_MODIFIER(p_func_Ret, md);
            APPLY_ATTRIBUTES(p_func_Ret, attrs);
            return p_func_Ret.value();
        }
        else if (check(TokenType::kClass)) {
            // class decl parsing logic

            auto p_class_Ret = parseClass();
            CHECK_ERROR(p_class_Ret);
            APPLY_MODIFIER(p_class_Ret, md);
            APPLY_ATTRIBUTES(p_class_Ret, attrs);
            return p_class_Ret.value();
        }
        else if (check(TokenType::kStruct)) {
            // struct decl parsing logic

            auto p_struct_Ret = parseStruct();
            CHECK_ERROR(p_struct_Ret);
            APPLY_MODIFIER(p_struct_Ret, md);
            APPLY_ATTRIBUTES(p_struct_Ret, attrs);
            return p_struct_Ret.value();
        }
        else if (check(TokenType::kInterface)) {
            // interfacce decl parsing logic

            auto p_intf_Ret = parseInterface();
            CHECK_ERROR(p_intf_Ret);
            APPLY_MODIFIER(p_intf_Ret, md);
            APPLY_ATTRIBUTES(p_intf_Ret, attrs);
            return p_intf_Ret.value();
        }
        else if (check(TokenType::kEnum)) {
            // enum decl parsing logic

            auto p_enum_Ret = parseEnum();
            CHECK_ERROR(p_enum_Ret);
            APPLY_MODIFIER(p_enum_Ret, md);
            APPLY_ATTRIBUTES(p_enum_Ret, attrs);
            return p_enum_Ret.value();
        }
        else if (check(TokenType::kVal) || check(TokenType::kVar)) {
            // variable parsing

            auto p_var_Ret = parseVarDecl();
            CHECK_ERROR(p_var_Ret);
            APPLY_MODIFIER(p_var_Ret, md);
            APPLY_ATTRIBUTES(p_var_Ret, attrs);
            return p_var_Ret.value();
        }
        else {
            return Result::failure(msg("unexpected token '", current().typeString(), "' found"), ERRR());
        }
    }

    // class parser
    // syntax like [xxx(xxx)] xxx class xxx : xxx { ... }
    Expected<ClassDecl*> NParser::parseClass()
    {
        if (!check(TokenType::kClass)) {
            return nullptr;
        }
        advance();

        // class name parsing
        if (!check(TokenType::kIdentifier)) {
            return Result::failure(msg("expected identifier for class name but got : '", current().typeString(), "'"), ERRR());
        }
        std::string name = current().value;

        // super classes parsing
        std::vector<ASTTypeNode*> baseClasses{};
        auto gd = ScopeGuard<ClassDecl>(new ClassDecl(name, {}));

        // pre-def for body parsing
        std::vector<Attribute*> attrs {};
        ASTModifier md {};

        if (check(TokenType::kColon)) {
            // parse base class types

            do {
                advance();
                if (check(TokenType::kLBraces) || check(TokenType::kSemicolon)) {
                    break;
                }
                else if (check(TokenType::kComma)) {
                    advance();
                    break;
                }
                else {
                    auto tp = parseType();
                    CHECK_ERROR(tp);
                    baseClasses.push_back(tp.value());
                }
            } while(true);
            gd->baseClasses = std::move(baseClasses);

            if (check(TokenType::kLBraces)) {
                goto parseBody;
            }
            else if (check(TokenType::kSemicolon)) {
                goto end;
            }
            else {
                return Result::failure(msg("unexpect token '", current().typeString(), "' for class declare"), ERRR());
            }
        }
        else if (check(TokenType::kLBraces)) {
            goto parseBody;
        }
        else if (check(TokenType::kSemicolon)) {
            goto end;
        }
        else {
            return Result::failure(msg("unexpect token '", current().typeString(), "' for class declare"), ERRR());
        }

    parseBody:
        // class body parsing

        do {
            advance();

            if (check(TokenType::kLBracket)) {
                // attributes parsing

                auto rq = parseAttributes();
                CHECK_ERROR(rq);
                attrs = rq.value();
            }
            else if (check(TokenType::kIdentifier)) {
                // modifier parsing

                auto r = parseModifier();
                CHECK_ERROR(r);
                md = r.value();
            }
            else if (check(TokenType::kFun) || check(TokenType::kDtor) || check(TokenType::kCtor)) {
                // class constructor / destructor / normal function parsing

                byte type = check(TokenType::kCtor) ? 1 : (check(TokenType::kDtor) ? 2 : 0);
                auto r = parseFunc();
                CHECK_ERROR(r);
                APPLY_MODIFIER_RAW(r, md);
                APPLY_ATTRIBUTES(r, attrs);
                if (type == 1) {
                    gd->ctors.push_back(r.value());
                } else if (type == 2) {
                    if (gd->dtors != nullptr)
                        return Result::failure("redefined destructor.", ERRR());
                    gd->dtors = r.value();
                } else {
                    gd->functions.push_back(r.value());
                }
            }
            else if (check(TokenType::kField)) {
                // class field parsing

                auto r = parseField();
                CHECK_ERROR(r);
                APPLY_MODIFIER_RAW(r, md);
                APPLY_ATTRIBUTES(r, attrs);
                gd->fields.push_back(r.value());
            }
            else if (check(TokenType::kRBraces)) {
                // end scope parsed

                break;
            }
            else {
                auto dr = parseDecl();
                CHECK_ERROR(dr);
                APPLY_MODIFIER_RAW(dr, md);
                APPLY_ATTRIBUTES(dr, attrs);
                auto* dk = dr.value();

                // sub-data-types
                if (auto* dClass = dynamic_cast<ClassDecl*>(dk)) {
                    gd->subDataTypes.push_back(dClass);
                }
                else if (auto* dStruct = dynamic_cast<StructDecl*>(dk)) {
                    gd->subDataTypes.push_back(dStruct);
                }
                else if (auto* dIntf = dynamic_cast<InterfaceDecl*>(dk)) {
                    gd->subDataTypes.push_back(dIntf);
                }
                else if (auto* dEnum = dynamic_cast<EnumDecl*>(dk)) {
                    gd->subDataTypes.push_back(dEnum);
                }
                // variabled
                else if (auto* dVar = dynamic_cast<VarDecl*>(dk)) {
                    gd->variables.push_back(dVar);
                }
                else {
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
    Expected<VarDecl*> NParser::parseVarDecl() {
        if (!check(TokenType::kVal) && !check(TokenType::kVar)) {
            return nullptr;
        }
        bool isNonChanged = check(TokenType::kVal);
        advance();

        // parse variable name
        if (!check(TokenType::kIdentifier)) {
            return Result::failure("unexpected token found after var/val : var xxx <--", ERRR());
        }
        std::string_view name = current().value;
        advance();
        auto gd = ScopeGuard(new VarDecl(name, nullptr));

        if (check(TokenType::kColon)) {
            // parse type hint

            advance();
            auto r = parseType();
            CHECK_ERROR(r);
            gd->type = r.value();

            // check assign
            if (check(TokenType::kEq)) {
                goto parseAssign;
            }
        }
        else if (check(TokenType::kEq)) {
            // parse assign expression
            parseAssign:
            advance();
            auto r = parseExpr();
            CHECK_ERROR(r);
            gd->initExpr = r.value();

            // check end of line
            if (!check(TokenType::kSemicolon)) {
                return Result::failure("expression line is not closed : var xxx = xxx <-- need ';' to closed", ERRR());
            }
        }
        else {
            return Result::failure("variable declare without type hint is not allow! var xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // enum parser
    // syntax like 'enum XXX : XXX { ... }'
    Expected<EnumDecl*> NParser::parseEnum() {
        if (!check(TokenType::kEnum)) {
            return nullptr;
        }
        advance();

        // parse enum name
        if (!check(TokenType::kIdentifier)) {
            return Result::failure("unexpected token after enum token : enum xxx <--", ERRR());
        }
        std::string_view name = current().value;
        advance();

        auto gd = ScopeGuard(new EnumDecl(name));

        if (check(TokenType::kColon)) {
            // parse enum base type

            advance();
            auto t = parseType();
            CHECK_ERROR(t);
            gd->baseType = t.value();

            if (check(TokenType::kLBraces)) {
                goto parseBody;
            }
            else if (check(TokenType::kSemicolon)) {
                // head only declare

                advance();
                return gd.getPtr();
            }
            else {
                return Result::failure("unexpected token after enum head declare : enum xxx : xxx ... <--", ERRR());
            }
        }
        else if (check(TokenType::kLBraces)) {
            // parse enum body

            parseBody:

            do {
                advance();
                if (check(TokenType::kIdentifier)) {
                    std::string_view itemName = current().value;

                    advance();
                    if (check(TokenType::kEq)) {
                        // parse enum assignment

                        advance();
                        auto eas = parseExpr();
                        CHECK_ERROR(eas);

                        // check next
                        if (check(TokenType::kComma)) {
                            advance();
                        }
                        gd->children.push_back(new VarDecl(itemName, nullptr, eas.value()));
                    }
                    else if (check(TokenType::kComma)) {
                        gd->children.push_back(new VarDecl(itemName, nullptr));
                        continue;
                    }
                    else {
                        return Result::failure("Invalied expression in enum body", ERRR());
                    }
                }
                else if (check(TokenType::kRBraces)) {
                    // end scope

                    break;
                }
                else if (check(TokenType::kComma)) {
                    advance();
                }
            } while(true);
        }
        else {
            return Result::failure("unexpected token after enum head declare : enum xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // field parser
    // syntax like 'field xxx : xxx {XXX,XXX} = xxx;'
    Expected<FieldDecl *> NParser::parseField() {
        if (!check(TokenType::kField)) {
            return nullptr;
        }
        advance();

        if (!check(TokenType::kIdentifier)) {
            return Result::failure("unexpected token after field keyword : field xxx <--", ERRR());
        }
        std::string_view name = current().value;
        auto gd = ScopeGuard(new FieldDecl(name, nullptr));

        advance();
        if (check(TokenType::kColon)) {
            // parse type hint

            advance();
            auto thr = parseType();
            CHECK_ERROR(thr);
            gd->type = thr.value();

            // check body
            if (!check(TokenType::kLBraces)) {
                return Result::failure("unexpected token after field's type hint : field xxx : xxx ... <--", ERRR());
            }
            goto parseBody;
        }
        else if (check(TokenType::kLBraces)) {
            parseBody:

            advance();
            // check read function name
            if (check(TokenType::kIdentifier)) {
                std::string_view funcName = current().value;
                gd->getFuncName = funcName;
                advance();

                if (!check(TokenType::kComma)) {
                    return Result::failure("unexpected token after field's read function : field xxx {XXX ... <--", ERRR());
                }
                advance();
            } else if (check(TokenType::kComma)) {
                advance();
            } else {
                return Result::failure("unexpected token in field's body : field xxx {... <--", ERRR());
            }

            // check write function name
            if (check(TokenType::kIdentifier)) {
                std::string_view funcName = current().value;
                gd->setFuncName = funcName;
                advance();

                if (!check(TokenType::kRBraces)) {
                    return Result::failure("unexpected token after field's write function : field xxx {XXX,XXX... <--", ERRR());
                }
                advance();
            } else if (check(TokenType::kRBraces)) {
                advance();
            } else {
                return Result::failure("unexpected token in field's body : field xxx {XXX,... <--", ERRR());
            }

            if (check(TokenType::kEq)) {
                // parse assign expression

                advance();
                auto iexp = parseExpr();
                CHECK_ERROR(iexp);
                gd->init = iexp.value();

                if (!check(TokenType::kSemicolon)) {
                    goto errorBc;
                }
                advance();
            }
            else if (check(TokenType::kSemicolon)) {
                advance();
            }
            else {
                errorBc:
                return Result::failure("unexpected token after field declare expression : field xxx {XXX,XXX} = XXX... <--", ERRR());
            }
        }
        else {
            return Result::failure("unexpected token after field's name : field XXX ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // Statement parser
    // syntax ...
    Expected<ASTStmt*> NParser::parseStmt() {
        ASTStmt* result = nullptr;

        switch (current().type)
        {
            case TokenType::kIf:
                break;
            case TokenType::kWhile:
                break;
            case TokenType::kFor:
                break;
            case TokenType::kReturn:
                break;
            case TokenType::kContinue:
                break;
            case TokenType::kTry:
                break;
            case TokenType::kThrow:
                break;
            case TokenType::kBreak:
                break;

            default:
                // fallback : decl parsing
                auto rDecl = parseDecl();
                CHECK_ERROR(rDecl);
                if (rDecl.value() == nullptr) {
                    // fallback : expression statement

                    auto rExpr = parseExpr();
                    CHECK_ERROR(rExpr);
                    if (!check(TokenType::kSemicolon)) {
                        return Result::failure("expression line should end with semi-colon ';' ", ERRR());
                    }
                    result = new ExprStmt(rExpr.value());
                } else {
                    result = new DeclStmt(rDecl.value());
                }
                break;
        }

        return result;
    }

    Expected<InterfaceDecl*> NParser::parseInterface() {
        if (check(TokenType::kInterface)) {
            return nullptr;
        }
        advance();

        // parse interface's name
        if (!check(TokenType::kIdentifier)) {
            return Result::failure("unexpected token after interface keyword : interface ... <--", ERRR());
        }
        std::string_view name = current().value;
        auto gd = ScopeGuard(new InterfaceDecl(name));
        advance();

        if (check(TokenType::kLBraces)) {
            // parse interface body

            do {
                advance();
                if (!check(TokenType::kIdentifier)) {
                    return Result::failure("unexpected identifier in interface body", ERRR());
                }

                ASTModifier md {};
                auto r = parseModifier();
                CHECK_ERROR(r);
                md = r.value();

                std::string_view func_name = current().value;
                advance();


            } while(true);
        } else if (check(TokenType::kSemicolon)) {
            // parse interface defination

            advance();
        } else {
            return Result::failure("unexpected token after interface's name : interface xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

    Expected<StructDecl *> NParser::parseStruct() {
        return nullptr;
    }

    Expected<ASTExpr*> NParser::parseExpr() {
        return nullptr;
    }


    bool NParser::parse()
    {
        auto& output = m_args.output;
        output.clearNodes();

        // parse entry
        auto r = parseRoot();
        if (!r) {
            m_diag.printAll();
            return false;
        }

        // dump ast logic

        return true;
    }

#if NE_DEBUG
    bool NParser::debugParse() {
        auto& output = m_args.output;
        output.clearNodes();

        // parse entry
        auto r = parseRoot();
        if (!r) {
            m_diag.printAll();
            return false;
        }

        // dump ast logic

        return true;
    }



#endif

}