// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>

#include "neo/ast/Type.hpp"
#include "neo/ast/Stmts.hpp"
#include "neo/diagnose/Diagnostic.hpp"
#include "neo/compiler/Tokens.hpp"

#include "neo/ast/Base.hpp"
#include "neo/ast/Decl.hpp"

namespace neo {

    struct NParserArgs
    {
        class NLexer* lexer;
        class NSourceFile* file;
        class NParsedFile& output;
        i32 langVer;
    };


#define APPLY_MODIFIER(V, MD) do { \
        V->modifier = MD.value();     \
   } while(false)
#define APPLY_MODIFIER_RAW(V, MD) do { \
        V->modifier = std::move(MD);   \
        MD = ASTModifier {};           \
   } while(false)
#define APPLY_ATTRIBUTES(V, AT) do { \
        V->attributes = std::move(AT); \
        AT = std::vector<Attribute*>();\
    } while(false)


    class NParser
    {
    public:
        NParser(NParserArgs args);
        ~NParser();

    public:
        bool parse();
        void debugPrint(class NDebugOutput&);

#if NE_DEBUG
        bool debugParse();
#endif

    private:
        NToken& current();
        NToken& peek();
        NToken& peekPrevious();
        NToken& advance();
        NToken& previous();
        bool match(TokenType);
        bool expect(TokenType);
        bool check(TokenType);

	private:
		bool isModifier(TokenType);
		bool isType();

    private:
        Expected<void> parseRoot();

        Expected<ASTStmt*> parseStmt();
        Expected<ASTExpr*> parseExpr();

        Expected<ASTTypeNode*> parseType();

        Expected<ImportStmt*> parseImport();
        Expected<ModuleDecl*> parseModule();

        Expected<ASTDecl*> parseDecl();
        Expected<CompoundStmt*> parseScope();

        Expected<ClassDecl*> parseClass();
        Expected<EnumDecl*> parseEnum();
        Expected<InterfaceDecl*> parseInterface();
        Expected<StructDecl*> parseStruct();
        Expected<VarDecl*> parseVarDecl();

        Expected<FieldDecl*> parseField();

        Expected<std::vector<Attribute*>> parseAttributes();
        Expected<ASTModifier> parseModifier();

        Expected<FuncDecl*> parseFunc(bool isLambda = false);
        Expected<std::vector<VarDecl*>> parseFuncArgs();
        Expected<std::vector<ASTExpr*>> parseFuncCallArgs();
		Expected<std::vector<ASTTypeNode*>> parseParents();

		Expected<IfStmt*> parseIfStmt(bool onlyIf = false);
		Expected<ReturnStmt*> parseReturnStmt();
		Expected<TryStmt*> parseTryCatch();
		Expected<ForStmt*> parseForStmt();
		Expected<WhileStmt*> parseWhileStmt();

		Expected<ASTExpr*> parseAssignExpr();
		Expected<ASTExpr*> parseLogicalOrExpr();
		Expected<ASTExpr*> parseLogicalAndExpr();
		Expected<ASTExpr*> parseEqualityExpr();
		Expected<ASTExpr*> parseRelationalExpr();
		Expected<ASTExpr*> parseAdditiveExpr();
		Expected<ASTExpr*> parsePostfixExpr();
		Expected<ASTExpr*> parseMultiplicativeExpr();
		Expected<ASTExpr*> parseUnaryExpr();
		Expected<ASTExpr*> parsePrimaryExpr();

    private:
        NParserArgs m_args;
        DiagnosticCollector m_diag;
        NLexer* m_lexer;

        static TokenType s_modifier[8];
        static TokenType s_declStmt[];
    };
}