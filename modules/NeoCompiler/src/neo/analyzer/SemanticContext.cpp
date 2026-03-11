/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "SemanticContext.hpp"

#include <nbase/memory/Memory.hpp>

namespace neo
{
    SemanticContext::SemanticContext(DiagnosticCollector *diag)
        : diagnostics{diag}
    {
        initBuiltins();
        globalScope = createScope(ScopeKind::kGlobal);
        currentScope = globalScope;
    }

    SemanticContext::~SemanticContext()
    {
        for (auto *scope : m_ownedScopes)
        {
            neo::deletePtr(scope);
        }
        for (auto *symbol : m_ownedSymbols)
        {
            neo::deletePtr(symbol);
        }
        for (auto *type : m_ownedTypes)
        {
            neo::deletePtr(type);
        }
    }

    Scope *SemanticContext::createScope(ScopeKind kind, ASTNode *owner, Scope *parent)
    {
        auto *scope = new Scope(kind, owner, parent == nullptr ? currentScope : parent);
        m_ownedScopes.push_back(scope);
        return scope;
    }

    Symbol *SemanticContext::ownSymbol(Symbol *symbol)
    {
        if (symbol != nullptr)
        {
            m_ownedSymbols.push_back(symbol);
        }
        return symbol;
    }

    SemanticType *SemanticContext::ownType(SemanticType *type)
    {
        if (type != nullptr)
        {
            m_ownedTypes.push_back(type);
        }
        return type;
    }

    void SemanticContext::pushScope(Scope *scope)
    {
        if (scope != nullptr)
        {
            currentScope = scope;
        }
    }

    void SemanticContext::popScope()
    {
        if (currentScope != nullptr && currentScope->parent != nullptr)
        {
            currentScope = currentScope->parent;
        }
    }

    void SemanticContext::bindDeclSymbol(const ASTDecl *decl, Symbol *symbol)
    {
        if (decl != nullptr && symbol != nullptr)
        {
            m_declSymbols[decl] = symbol;
        }
    }

    void SemanticContext::bindExprType(const ASTExpr *expr, SemanticType *type)
    {
        if (expr != nullptr && type != nullptr)
        {
            m_exprTypes[expr] = type;
        }
    }

    void SemanticContext::bindResolvedSymbol(const ASTExpr *expr, Symbol *symbol)
    {
        if (expr != nullptr && symbol != nullptr)
        {
            m_exprSymbols[expr] = symbol;
        }
    }

    void SemanticContext::bindResolvedType(const ASTTypeNode *node, SemanticType *type)
    {
        if (node != nullptr && type != nullptr)
        {
            m_resolvedTypes[node] = type;
        }
    }

    Symbol *SemanticContext::getDeclSymbol(const ASTDecl *decl) const
    {
        auto it = m_declSymbols.find(decl);
        return it == m_declSymbols.end() ? nullptr : it->second;
    }

    SemanticType *SemanticContext::getExprType(const ASTExpr *expr) const
    {
        auto it = m_exprTypes.find(expr);
        return it == m_exprTypes.end() ? nullptr : it->second;
    }

    Symbol *SemanticContext::getResolvedSymbol(const ASTExpr *expr) const
    {
        auto it = m_exprSymbols.find(expr);
        return it == m_exprSymbols.end() ? nullptr : it->second;
    }

    SemanticType *SemanticContext::getResolvedType(const ASTTypeNode *node) const
    {
        auto it = m_resolvedTypes.find(node);
        return it == m_resolvedTypes.end() ? nullptr : it->second;
    }

    void SemanticContext::initBuiltins()
    {
        builtins.typeError = static_cast<ErrorType *>(ownType(new ErrorType()));
        builtins.typeVoid = static_cast<BuiltinType *>(ownType(new BuiltinType(SemanticTypeKind::kVoid, "void")));
        builtins.typeBool = static_cast<BuiltinType *>(ownType(new BuiltinType(SemanticTypeKind::kBool, "bool")));
        builtins.typeInt = static_cast<BuiltinType *>(ownType(new BuiltinType(SemanticTypeKind::kInt, "i32")));
        builtins.typeFloat = static_cast<BuiltinType *>(ownType(new BuiltinType(SemanticTypeKind::kFloat, "f32")));
        builtins.typeString = static_cast<BuiltinType *>(ownType(new BuiltinType(SemanticTypeKind::kString, "string")));
        builtins.typeNull = static_cast<BuiltinType *>(ownType(new BuiltinType(SemanticTypeKind::kNull, "null")));
    }
}
