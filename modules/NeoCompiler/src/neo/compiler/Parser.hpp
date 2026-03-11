/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/common.hpp>
#include <initializer_list>

#include "neo/ast/Type.hpp"
#include "neo/ast/Stmts.hpp"
#include "neo/ast/Exprs.hpp"
#include "neo/diagnose/Diagnostic.hpp"
#include "neo/compiler/Tokens.hpp"

#include "neo/ast/Base.hpp"
#include "neo/ast/Decl.hpp"

namespace neo
{
    struct NParserArgs
    {
        class NLexer *lexer;
        class NSourceFile *file;
        class NParsedFile &output;
        i32 langVer;
    };

#define APPLY_MODIFIER(V, MD)     \
    do                            \
    {                             \
        auto *_node = (V).value(); \
        if (_node != nullptr)     \
        {                         \
            _node->modifier = MD.value(); \
        }                         \
    } while (false)
#define APPLY_MODIFIER_RAW(V, MD)    \
    do                               \
    {                                \
        if ((V) != nullptr)          \
        {                            \
            (V)->modifier = std::move(MD); \
        }                            \
        MD = ASTModifier{};          \
    } while (false)
#define APPLY_ATTRIBUTES(V, AT)        \
    do                                 \
    {                                  \
        auto *_node = (V).value();     \
        if (_node != nullptr)          \
        {                              \
            _node->attributes = std::move(AT); \
        }                              \
        AT = Vector<Attribute *>();    \
    } while (false)

    class NParser final
    {
    public:
        NParser(NParserArgs args);
        ~NParser();

    public:
        bool parse();
        void debugPrint(class NDebugOutput &);

#if NE_DEBUG
        bool debugParse();
#endif

    private:
        NToken &current();
        NToken &peek();
        NToken &peekPrevious();
        NToken &advance();
        NToken &previous();
        bool match(TokenType);
        bool expect(TokenType);
        bool check(TokenType);
        bool isTypeAt(psize idx, psize *endIdx = nullptr) const;
        String tokenText(const NToken &token) const;
        Result unexpectedToken(const String &context);
        Result expectedToken(const String &expected, const String &context);
        void synchronize(std::initializer_list<TokenType> tokens, bool consumeToken = false);
        void synchronizeTopLevel();
        void synchronizeStmt();
        void synchronizeExpr();
        bool shouldAbort() const;
        ASTExpr *makeErrorExpr(const NToken &start);
        ASTStmt *makeErrorStmt(const NToken &start);
        ASTDecl *makeErrorDecl(const NToken &start);

    private:
        bool isModifier(TokenType);
        bool isType();

    private:
        Expected<void> parseRoot();

        Expected<ASTStmt *> parseStmt();
        Expected<ASTExpr *> parseExpr(bool skipCommaExpr = true);

        Expected<ASTTypeNode *> parseType();

        Expected<ImportDecl *> parseImport();
        Expected<ModuleDecl *> parseModule();

        Expected<ASTDecl *> parseDecl();
        Expected<CompoundStmt *> parseScope();

        Expected<ClassDecl *> parseClass();
        Expected<EnumDecl *> parseEnum();
        Expected<InterfaceDecl *> parseInterface();
        Expected<StructDecl *> parseStruct();
        Expected<VarDecl *> parseVarDecl(bool consumeTerminator = true);
        Expected<VarDecl *> parseTypedVarDecl(bool consumeTerminator = true);
        Expected<VarDecl *> parseColonVarDecl(bool consumeTerminator = true);

        Expected<FieldDecl *> parseField();

        Expected<Vector<Attribute *>> parseAttributes();
        Expected<ASTModifier> parseModifier();
        Expected<void> parseGenericSuffix(String &out, Vector<GenericParamDecl *> *params = nullptr);

        Expected<FuncDecl *> parseFunc(bool isLambda = false);
        Expected<Vector<VarDecl *>> parseFuncArgs();
        Expected<Vector<ASTExpr *>> parseFuncCallArgs();
        Expected<Vector<ASTTypeNode *>> parseParents();

        Expected<IfStmt *> parseIfStmt(bool onlyIf = false);
        Expected<ReturnStmt *> parseReturnStmt();
        Expected<TryStmt *> parseTryCatch();
        Expected<ForStmt *> parseForStmt();
        Expected<WhileStmt *> parseWhileStmt();

        Expected<ASTExpr *> parseAssignExpr();

        Expected<ASTExpr *> parseLogicalOrExpr();
        Expected<ASTExpr *> parseLogicalAndExpr();
        Expected<ASTExpr *> parseBitwiseOrExpr();
        Expected<ASTExpr *> parseBitwiseXorExpr();
        Expected<ASTExpr *> parseBitwiseAndExpr();
        Expected<ASTExpr *> parseEqualityExpr();
        Expected<ASTExpr *> parseRelationalExpr();
        Expected<ASTExpr *> parseShiftExpr();
        Expected<ASTExpr *> parseAdditiveExpr();
        Expected<ASTExpr *> parseMultiplicativeExpr();
        Expected<ASTExpr *> parseUnaryExpr();
        Expected<ASTExpr *> parsePostfixExpr();
        Expected<ASTExpr *> parsePrimaryExpr();
        Expected<ASTExpr *> parseConditionalExpr();
        Expected<ASTExpr *> parseCommaExpr();

    private:
        Expected<ASTExpr *> parseBinaryExpr(
            Expected<ASTExpr *> (NParser::*subExpr)(),
            const Vector<std::pair<TokenType, BinaryOp>> &ops);

        template <typename T>
        T *setLoc(T *node, const NToken &token)
        {
            if (node)
            {
                node->m_loc = token.location(m_args.file);
            }
            return node;
        }

    private:
        NParserArgs m_args;
        DiagnosticCollector m_diag;
        NLexer *m_lexer;
        Vector<String> m_activeGenericTypeNames;

        static TokenType s_modifier[11];
        static TokenType s_declStmt[];
        static constexpr int s_maxRecoverErrors = 20;
    };
}
