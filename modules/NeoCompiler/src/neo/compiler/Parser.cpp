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
#include "neo/ast/Exprs.hpp"

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
    Expected<FuncDecl*> NParser::parseFunc(bool isLambda)
    {
        if (!check(TokenType::kFun) && expect(TokenType::kIdentifier)) {
            return Result::failure(msg("unexpected token for function declare : ", current().value, " ", peek().value), ERRR());
        }
        advance();
	    std::string name{};

		if (!isLambda)
		{
			// function name parsing logic
			name = current().value;
			advance(); // Skip name
		}

	    if (!check(TokenType::kLParen)) {
		    return Result::failure(msg("function declare expect '(' for function arguments but got '", current().value, "'"), ERRR());
	    }

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
	//
    Expected<std::vector<ASTExpr*>> NParser::parseFuncCallArgs()
    {
        std::vector<ASTExpr*> args{};
        if (!check(TokenType::kLParen)) {
            return args;
        }

        do {
            advance();
            if (check(TokenType::kComma)) {
                advance(); // Skip ','
            } else if (check(TokenType::kRParen)) {
                advance(); // Skip ')'
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
    Expected<std::vector<VarDecl*>> NParser::parseFuncArgs()
	{
        // function argument parsing logic;

		std::vector<VarDecl*> args{};
        std::vector<Attribute*> attrs{};

	    if (!check(TokenType::kLParen))
		    return args;
        advance(); // Skip '('
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

                advance(); // Skip ','
                continue;
            } else if (check(TokenType::kRParen)) {
                // end loop when matched ')'

                advance(); // Skip ')'
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
	        advance(); // Skip name

            // check '[xxx(' <- and parse args
			if (check(TokenType::kRBracket)) {
				attrs.push_back(g.getPtr());
				advance(); // Skip ']'
				continue;
			}
			else if (check(TokenType::kLParen)) {
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
            else {
                CLEARUP(attrs);
                return Result::failure(msg("unexpect token '", current().value, "' after attribute attach's name"), ERRR());
            }
        } while (check(TokenType::kLBracket));
		auto c = current();

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
        auto gd = ScopeGuard<ClassDecl>(new ClassDecl(name, {}));

        // pre-def for body parsing
        std::vector<Attribute*> attrs {};
        ASTModifier md {};

        if (check(TokenType::kColon)) {
            // parse base class types

            auto pR = parseParents();
			CHECK_ERROR(pR);
            gd->baseClasses = std::move(pR.value());

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

            advance(); // EAT Colon
            auto r = parseType();
            CHECK_ERROR(r);
            gd->type = r.value();

            // check assign
            if (check(TokenType::kEq)) {
                goto parseAssign;
            }
			// EAT Semicolon
			else if (check(TokenType::kSemicolon)) {
				advance();
			}
			else {
				return Result::failure("expression is not closed : var xxx : xxx <-- need ';' to close the line", ERRR());
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
                return Result::failure("expression line is not closed : var xxx = xxx <-- need ';' to close the line", ERRR());
            }
			advance(); // EAT Semicolon
        }
        else {
            return Result::failure("variable declare without type hint is not allow! var xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

    // enum parser
    // syntax like 'enum XXX : XXX { ... }'
    Expected<EnumDecl*> NParser::parseEnum()
	{
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
    Expected<ASTStmt*> NParser::parseStmt()
	{
        ASTStmt* result = nullptr;

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

				result = new ContinueStmt();

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
				advance(); //Skip 'throw'

				auto rTro = parseExpr();
				CHECK_ERROR(rTro);
				result = new ThrowStmt(rTro.value());

				break;
			}
            case TokenType::kBreak:
            {
				// Parse break statement.
				advance(); // Skip 'break'
				result = new BreakStmt{};
	            break;
            }
            default:
            {
	            // fallback : decl parsing
	            auto rDecl = parseDecl();
	            CHECK_ERROR(rDecl);
	            if (rDecl.value() == nullptr)
	            {
		            // fallback : expression statement

		            auto rExpr = parseExpr();
		            CHECK_ERROR(rExpr);
		            if (!check(TokenType::kSemicolon))
		            {
			            return Result::failure("expression line should end with semi-colon ';' ", ERRR());
		            }
		            result = new ExprStmt(rExpr.value());
	            }
	            else
	            {
		            result = new DeclStmt(rDecl.value());
	            }
	            break;
            }
        }

        return result;
    }

	// Interface parser
	// Syntax like : [...] interface XXX { ... }
    Expected<InterfaceDecl*> NParser::parseInterface() {
        if (!check(TokenType::kInterface)) {
            return nullptr;
        }
        advance(); // Skip 'interface'

        // parse interface's name
        if (!check(TokenType::kIdentifier)) {
            return Result::failure("unexpected token after interface keyword : interface ... <--", ERRR());
        }
        auto gd = ScopeGuard(new InterfaceDecl(current().value));
        advance(); // Skip name

        if (check(TokenType::kLBraces)) {
            // Parse interface's body
	        advance(); // Skip '{'

			ASTModifier md {};
			std::vector<Attribute*> attr {};

            do {
				if (check(TokenType::kLBracket)) {
					// Parse Attribute

					auto rA = parseAttributes();
					CHECK_ERROR(rA);
					attr = rA.value();

				} else if (isModifier(current().type)) {
					// Parse modifier if matched.

					auto r = parseModifier();
					CHECK_ERROR(r);
					md = r.value();

				} else if (check(TokenType::kIdentifier)) {
					// Parse function item

					std::string_view func_name = current().value;
					advance(); // Skip function's name

					if (!check(TokenType::kLParen)) {
						return Result::failure("unexpected token after function's name", ERRR());
					}
					auto rAgs = parseFuncArgs();
					CHECK_ERROR(rAgs);
					auto fnc = new FuncDecl();
					fnc->name = func_name;
					fnc->modifier = std::move(md);
					md = ASTModifier {};
					fnc->attributes = std::move(attr);
					attr = std::vector<Attribute*> {};

					gd->children.push_back(fnc);

					if (!check(TokenType::kSemicolon)) {
						return Result::failure("function declare was not closed", ERRR());
					}
					advance(); // Skip ';'

				} else {
                    return Result::failure("unexpected identifier in interface body", ERRR());
                }
            } while(true);

        } else if (check(TokenType::kSemicolon)) {
            // Parse interface defination
            advance(); // Skip ';'

        } else {
            return Result::failure("unexpected token after interface's name : interface xxx ... <--", ERRR());
        }

        return gd.getPtr();
    }

	// Struct parser
	// Syntax like : [...] struct XXX { ... }
    Expected<StructDecl *> NParser::parseStruct() {
		if (!check(TokenType::kStruct))
			return nullptr;
		advance(); // Skip 'struct'

		// parse struct's name
		if (!check(TokenType::kIdentifier)) {
			return Result::failure("unexpected token after struct keyword : struct ... <--", ERRR());
		}
	    auto gd = ScopeGuard(new StructDecl(current().value));
		advance(); // Skip name

		if (check(TokenType::kColon)) {
			// Fallback:
			return Result::failure("Struct not support with parent classes!", ERRR());
		} else if (check(TokenType::kLBraces)) {
			// Parse body
			advance(); // Skip '{'

			Attribute attr {};
			ASTModifier md {};

			do {
				if (check(TokenType::kVal) || check(TokenType::kVar)) {
					// Parse variable

					auto var = parseVarDecl();
					CHECK_ERROR(var);
					auto ptr = var.value();
					ptr->modifier = std::move(md);
					md = ASTModifier {};

					gd->variables.push_back(ptr);

					if (!check(TokenType::kComma)) {
						if (expect(TokenType::kRBraces))
							continue;
						return Result::failure("Need comma to split variable declaration.", ERRR());
					}
					advance(); // Skip ','

				} else if (isModifier(current().type)) {
					// Parse modifier if matched.

					auto rM = parseModifier();
					CHECK_ERROR(rM);
					md = rM.value();

				} else if (check(TokenType::kRBraces)) {
					// Break on '}'
					advance(); // Skip ';'
					break;
				} else {
					// Fallback:
					return Result::failure("Unexpected token found in struct's body.", ERRR());
				}
			}while(true);

		} else {
			return Result::failure("Unexpected token after struct's name", ERRR());
		}

		return gd.getPtr();
    }

	// If-Stmt parser
	// Syntax like : if (...) {...} else if (...) {...} else {...}
	Expected<IfStmt*> NParser::parseIfStmt(bool onlyIf)
	{
		if (!check(TokenType::kIf) && !expect(TokenType::kLParen))
			return nullptr;
		advance(); // Skip 'if'

		auto gd = ScopeGuard(new IfStmt());

		advance(); // Skip '('
		// Parse ifExpr
		auto rE = parseExpr();
		CHECK_ERROR(rE);
		gd->ifExpr = rE.value();

		if (!check(TokenType::kRParen))
			return Result::failure("If body's paren is not closed, need ')' after the body.", ERRR());
		advance(); // Skip ')'

		if (!check(TokenType::kLBraces)) {
			// Parse if branch's body

			auto rS = parseScope();
			CHECK_ERROR(rS);
			gd->defaultBranch = rS.value();

		} else {
			// Parse one line if statement.

			auto rB = parseExpr();
			CHECK_ERROR(rB);
			gd->defaultBranch = rB.value();

			if (!check(TokenType::kSemicolon))
				return Result::failure("Line not closed, need ';' after a statement.", ERRR());
			advance(); // Skip ';'
		}

		if (onlyIf)
			return gd.getPtr();

	checkEnd:
		if (check(TokenType::kElse)) {
			// Parse 'else'

			if (expect(TokenType::kIf)) {
				// Parse else if

				advance(); // Skip 'else'
				auto rN = parseIfStmt(true);
				CHECK_ERROR(rN);
				gd->elseIfBranches.push_back(rN.value());

				goto checkEnd; // Check if 'if' statements end
			}
			else if (expect(TokenType::kLBraces)) {
				// Parse else branch

				auto rS = parseScope();
				CHECK_ERROR(rS);
				gd->elseBranch = rS.value();

				goto checkEnd; // Check if 'if' statements end
			}
			else {
				// Parse one-line body

				auto rB = parseExpr();
				CHECK_ERROR(rB);
				gd->elseBranch = rB.value();

				if (!check(TokenType::kSemicolon))
					return Result::failure("Line not closed, need ';' after a statement.", ERRR());
				advance(); // Skip ';'

				goto checkEnd; // Check if 'if' statements end
			}
		} else {
			return gd.getPtr();
		}
	}

	// Return statement parser
	// Syntax like : return xxx;
	Expected<ReturnStmt*> NParser::parseReturnStmt()
	{
		if (!check(TokenType::kReturn))
			return nullptr;
		advance(); // Skip 'return'

		auto gd = ScopeGuard(new ReturnStmt());
		auto rE = parseExpr();
		CHECK_ERROR(rE);
		gd->ret = rE.value();

		if (!check(TokenType::kSemicolon))
			return Result::failure("return statement if not closed by semicolon, need ';' after return xxx <--", ERRR());
		return gd.getPtr();
	}

	// Base class parser
	// Syntax like: : XXX, XXX, ... { [or ;]
	Expected<std::vector<ASTTypeNode*>> NParser::parseParents()
	{
		std::vector<ASTTypeNode*> baseClasses;

		if (!check(TokenType::kColon))
			return baseClasses;

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

		return baseClasses;
	}

	// Try-catch parser
	// Syntax like: try {...} catch(...) {...} catch {...}
	Expected<TryStmt*> NParser::parseTryCatch()
	{
		// Check 'try {'
		if (!check(TokenType::kTry) || !expect(TokenType::kLBraces))
			return nullptr;
		advance(); // Skip 'try'

		auto rS = parseScope();
		CHECK_ERROR(rS);
		auto gd = ScopeGuard(new TryStmt(rS.value()));

	parseAgain:
		if (check(TokenType::kCatch)) {
			// Parse handler

			advance(); // Skip 'catch'
			auto gdH = ScopeGuard(new CatchStmt());
			gdH->errorType = nullptr;

			if (check(TokenType::kLParen)) {
				// Handler with type hint

				// Parse type
				auto rArgs = parseFuncArgs();
				CHECK_ERROR(rArgs);
				auto rD = rArgs.value();
				if (rD.size() != 1) {
					return Result::failure("Catch handler only avaliable for single exception type!", ERRR());
				}
				gdH->errorType = rD[0];
				goto parseBody;

			}
			else if (check(TokenType::kLBraces)) {
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
				return gd.getPtr();

		} else {
			return Result::failure("Try block without any catch handler", ERRR());
		}
	}

	// For loop parser
	// Syntax like: for(...;...;...) {...} or for {...}
	Expected<ForStmt*> NParser::parseForStmt()
	{
		if (!check(TokenType::kFor))
			return nullptr;
		advance(); // Skip 'for'

		auto gd = ScopeGuard(new ForStmt());

		if (check(TokenType::kLParen))
		{
			return Result::failure("for statement without loop expression.", ERRR());

			// Parse for's loop expression
			advance(); // Skip '('
			auto rVar = parseVarDecl();
			CHECK_ERROR(rVar);
			gd->declVar = rVar.value();

			if (!check(TokenType::kSemicolon))
				return Result::failure("expected ';' after loop variable declaration. var xxx = xxx <--", ERRR());
			advance(); // Skip ';'

			auto rExpr = parseExpr();
			CHECK_ERROR(rExpr);
			gd->cond = rExpr.value();

			if (!check(TokenType::kSemicolon))
				return Result::failure("expected ';' after loop condition expression. xxx != xxx <--", ERRR());
			advance(); // Skip ';'

			auto rUpdateExpr = parseExpr();
			CHECK_ERROR(rUpdateExpr);
			gd->update = rUpdateExpr.value();

			if (!check(TokenType::kRParen))
				return Result::failure("expected ')' after loop update expression. xxx... <--", ERRR());
			advance(); // Skip ')'

			if (check(TokenType::kLBraces)) {
				goto parseScopeBody;
			} else {
				// Parse one line for body statement.

				auto rB = parseExpr();
				CHECK_ERROR(rB);
				gd->forBody = rB.value();
			}

		} else if (check(TokenType::kLBraces)) {
			// Parse scoped statements for for

		parseScopeBody:
			auto rScp = parseScope();
			CHECK_ERROR(rScp);
			gd->forBody = rScp.value();

		} else {
			return Result::failure("Unexpected token after for token. ", ERRR());
		}

		return gd.getPtr();
	}

	// While loop parser
	// Syntax like: while(...) {...} or while {...}
	Expected<WhileStmt*> NParser::parseWhileStmt()
	{
		if (!check(TokenType::kWhile))
			return nullptr;
		advance(); // Skip 'while'
		auto gd = ScopeGuard(new WhileStmt());

		if (check(TokenType::kLParen)) {
			advance(); //Skip '('

			auto rExpr = parseExpr();
			CHECK_ERROR(rExpr);
			gd->condition = rExpr.value();

			if (!check(TokenType::kRParen))
				return Result::failure("while loop condition need closed by ')'.", ERRR());
			advance(); // Skip ')'

			if (check(TokenType::kLBraces)) {
				goto parseBody;
			}
			else {
				// Parse one line for body statement.

				auto rB = parseExpr();
				CHECK_ERROR(rB);
				gd->body = rB.value();
			}

		} else if (check(TokenType::kLBraces)) {
			// parse while body.

			parseBody:
			auto rScp = parseScope();
			CHECK_ERROR(rScp);
			gd->body = rScp.value();

		} else {
			return Result::failure("unexpected token after 'while' ", ERRR());
		}

		return gd.getPtr();
	}

	// Top level of expression parsing
    Expected<ASTExpr*> NParser::parseExpr() {
        return parseAssignExpr();
    }

	Expected<ASTExpr*> NParser::parseAssignExpr()
	{
		return Expected<ASTExpr*>(nullptr);
	}

	// Unary expression parser
	Expected<ASTExpr*> NParser::parseUnaryExpr()
	{
		if (match(TokenType::kAdd))
			return parseUnaryExpr(); // +(x) -> x

		if (match(TokenType::kLParen))
		{
			if (isType()) {
				// (T)a
				advance(); // Skip '('
				auto rType = parseType();
				if (!check(TokenType::kRParen))
					return Result::failure("type body is not closed -> ...)", ERRR());
				advance(); // Skip ')'
				auto rOperand = parseUnaryExpr();
				CHECK_ERROR(rOperand);
				return new CastExpr(rOperand.value(), rType.value());
			} else {
				previous();
				return parsePrimaryExpr();
			}
		}


		UnaryOp op = UnaryOp::kUnknown;
		switch (current().type) {
		case TokenType::kInc: op = UnaryOp::kPrePlus; break;
		case TokenType::kDec: op = UnaryOp::kPreMinus; break;
		case TokenType::kSub: op = UnaryOp::kMinus; break;
		case TokenType::kLNot: op = UnaryOp::kBang; break;
		case TokenType::kBitNot: op = UnaryOp::kTilde; break;
		case TokenType::kBitAnd: op = UnaryOp::kAmp; break;
		case TokenType::kMul: op = UnaryOp::kStar; break;
		default: break;
		}

		if (op != UnaryOp::kUnknown) {
			advance();
			auto operand = parseUnaryExpr();
			CHECK_ERROR(operand);
			return new UnaryExpr(op, operand.value());
		}

		return parsePrimaryExpr();
	}

	// Primary expression parser
	Expected<ASTExpr*> NParser::parsePrimaryExpr()
	{
		if (check(TokenType::kIdentifier))
		{
			// a b c
			auto rId = new ASTIdent(current().value);
			advance(); // Skip current token
			return rId;
		}
		else if (check(TokenType::kCharLit))
		{
			// 'a'
			auto rCLit = new CharLiteralExpr(current().value[0]);
			advance(); // Skip current token
			return rCLit;
		}
		else if (check(TokenType::kStringLit))
		{
			// "aaa"
			std::string value = current().value;

			// deal with "aaa""bbb" -> "aaabbb"
			processStringLit:
			if (expect(TokenType::kStringLit)) { // TIPS: expect won't cost token!!!
				advance();
				value.append(current().value);
				goto processStringLit;
			}

			auto rSLit = new StringLiteralExpr(value);
			advance(); // Skip current
			return rSLit;
		}
		else if (check(TokenType::kIntLit) || check(TokenType::kHexLit))
		{
			// 1 2 100 0x00
			auto rNLit = NumberLiteralExpr::parseNumberToken(current().value);
			CHECK_ERROR(rNLit);
			advance();
			return rNLit.value();
		}
		else if (check(TokenType::kFloatLit))
		{
			// 8.32
			auto rFLit = NumberLiteralExpr::parseFloatToken(current().value);
			CHECK_ERROR(rFLit);
			advance();
			return rFLit.value();
		}
		else if (check(TokenType::kLParen))
		{
			// (...)
			advance(); // Skip '('
			auto rInnerExpr = parseExpr();
			CHECK_ERROR(rInnerExpr);
			if (!check(TokenType::kRParen))
				return Result::failure("expression is not closed --> ')'", ERRR());
			advance(); // Skip ')'
			return rInnerExpr.value();
		}
		else if (check(TokenType::kTrue) || check(TokenType::kFalse))
		{
			// true false
			auto rBLit = new BoolLiteralExpr(current().type == TokenType::kTrue ? (bool)true : (bool)false);
			advance(); // Skip current
			return rBLit;
		}
		else if (check(TokenType::kNull))
		{
			// null
			advance(); // Skip current
			return NullExpr::getInstance();
		}
		else if (check(TokenType::kThis))
		{
			// null
			advance(); // Skip current
			return ThisExpr::getInstance();
		}
		else if (check(TokenType::kSuper))
		{
			// null
			advance(); // Skip current
			return SuperExpr::getInstance();
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
			return new NewExpr(rNType.value(), rNArgs.value());
		}
		else if (check(TokenType::kFun))
		{
			// parse lambda function
			auto rLFunc = parseFunc(true);
			CHECK_ERROR(rLFunc);
			return new LambdaFuncExpr(rLFunc.value());
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

			return new CastExpr(rCExpr.value(), rCType.value());
		}
		else if (check(TokenType::kLBracket))
		{
			// [a,b,c]
			advance(); // Skip '['
			std::vector<ASTExpr*> elements {};

			if (!check(TokenType::kRBracket))
			{
				while (true)
				{
					auto elem = parseExpr();
					CHECK_ERROR(elem);
					elements.push_back(elem.value());
					if (check(TokenType::kComma))
						advance(); // Skip ','
					else break;
				}
			}

			if (!check(TokenType::kRBracket))
				return Result::failure("array literal not closed -> ']'", ERRR());
			advance(); // skip ']'

			return new ArrayLiteralExpr(std::move(elements));
		}


		return Result::failure("Unexpected token in primary expression: " + current().toString(), ERRR());
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


	Expected<ASTExpr*> NParser::parseMultiplicativeExpr()
	{
		auto left = parseUnaryExpr();
		CHECK_ERROR(left);

		while(true) {
			BinaryOp op = BinaryOp::kUnknown;

			switch (current().type) {
			case TokenType::kMul:  op = BinaryOp::kMul;  break;  // *
			case TokenType::kDiv:  op = BinaryOp::kDiv;  break;  // /
			case TokenType::kMod:  op = BinaryOp::kMod;  break;  // %
			default:
				op = BinaryOp::kUnknown;
				break;
			}

			if (op == BinaryOp::kUnknown)
				break;

			advance(); // Skip operator

			auto right = parseUnaryExpr();
			CHECK_ERROR(right);

			left = new BinaryExpr(op, left.value(), right.value());
		}

		return left;
	}

	// Check current token is modifier or not
	bool NParser::isModifier(TokenType t)
	{
		if (std::find(&s_modifier[0], &s_modifier[7], t) == &s_modifier[7]) {
			return true;
		}
		return false;
	}

	// Check current is type or expression
	bool NParser::isType()
	{
		psize idx = m_lexer->m_tk_idx; // save current pos

		// check identifier
		if (idx >= m_lexer->m_tokens.size() || m_lexer->m_tokens[idx].type != TokenType::kIdentifier)
			return false;
		idx++;

		// deal with module path: a.b.c
		while (idx < m_lexer->m_tokens.size() && m_lexer->m_tokens[idx].type == TokenType::kDot) {
			idx++; // skip '.'
			if (idx >= m_lexer->m_tokens.size() || m_lexer->m_tokens[idx].type != TokenType::kIdentifier)
				return false; // '.' must be identifier
			idx++;
		}

		// pointer type
		while (idx < m_lexer->m_tokens.size() && m_lexer->m_tokens[idx].type == TokenType::kMul) {
			idx++;
		}

		// check array type: i32[10], i32[]
		if (idx < m_lexer->m_tokens.size() && m_lexer->m_tokens[idx].type == TokenType::kLBracket) {
			idx++;
			while (idx < m_lexer->m_tokens.size()) {
				auto t = m_lexer->m_tokens[idx].type;
				if (t == TokenType::kIntLit) {
					idx++;
					continue;
				} else if (t == TokenType::kComma) {
					idx++;
					continue;
				} else if (t == TokenType::kRBracket) {
					idx++;
					break;
				} else {
					return false;
				}
			}
		}

		// TODO: fall back
		if (idx < m_lexer->m_tokens.size()) {
			TokenType nt = m_lexer->m_tokens[idx].type;
			if (nt == TokenType::kIdentifier || nt == TokenType::kIntLit ||
				nt == TokenType::kFloatLit || nt == TokenType::kStringLit ||
				nt == TokenType::kLParen)
			{

			}
		}

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