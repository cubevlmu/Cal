/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/common.hpp>

#include "Scopes.hpp"
#include "Symbols.hpp"
#include "Types.hpp"
#include "neo/diagnose/Diagnostic.hpp"

namespace neo
{
    class ASTDecl;
    class ASTExpr;
    class ASTNode;
    class ASTTypeNode;

    struct BuiltinTypeSet
    {
        BuiltinType *typeVoid = nullptr;
        BuiltinType *typeBool = nullptr;
        BuiltinType *typeInt = nullptr;
        BuiltinType *typeFloat = nullptr;
        BuiltinType *typeString = nullptr;
        BuiltinType *typeNull = nullptr;
        ErrorType *typeError = nullptr;
    };

    class SemanticContext
    {
    public:
        explicit SemanticContext(DiagnosticCollector *diagnostics);
        ~SemanticContext();

    public:
        Scope *createScope(ScopeKind kind, ASTNode *owner = nullptr, Scope *parent = nullptr);
        Symbol *ownSymbol(Symbol *symbol);
        SemanticType *ownType(SemanticType *type);

        void pushScope(Scope *scope);
        void popScope();

        void bindDeclSymbol(const ASTDecl *decl, Symbol *symbol);
        void bindExprType(const ASTExpr *expr, SemanticType *type);
        void bindResolvedSymbol(const ASTExpr *expr, Symbol *symbol);
        void bindResolvedType(const ASTTypeNode *node, SemanticType *type);

        Symbol *getDeclSymbol(const ASTDecl *decl) const;
        SemanticType *getExprType(const ASTExpr *expr) const;
        Symbol *getResolvedSymbol(const ASTExpr *expr) const;
        SemanticType *getResolvedType(const ASTTypeNode *node) const;

    public:
        DiagnosticCollector *diagnostics = nullptr;
        Scope *globalScope = nullptr;
        Scope *currentScope = nullptr;
        FunctionSymbol *currentFunction = nullptr;
        TypeSymbol *currentType = nullptr;
        BuiltinTypeSet builtins;

    private:
        void initBuiltins();

    private:
        Vector<Scope *> m_ownedScopes;
        Vector<Symbol *> m_ownedSymbols;
        Vector<SemanticType *> m_ownedTypes;

        HashMap<const ASTDecl *, Symbol *> m_declSymbols;
        HashMap<const ASTExpr *, SemanticType *> m_exprTypes;
        HashMap<const ASTExpr *, Symbol *> m_exprSymbols;
        HashMap<const ASTTypeNode *, SemanticType *> m_resolvedTypes;
    };
}
