/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/memory/StlAllocator.hpp>

namespace neo
{
    class ASTNode;
    class Symbol;

    enum class ScopeKind
    {
        kUnknown,
        kGlobal,
        kModule,
        kType,
        kFunction,
        kLocal,
        kLoop,
        kGeneric,
    };
    StringView getTypeString(ScopeKind kind);

    class Scope
    {
    public:
        explicit Scope(ScopeKind kind, ASTNode *owner = nullptr, Scope *parent = nullptr);
        ~Scope() = default;

    public:
        ScopeKind getKind() const
        {
            return m_kind;
        }

        bool declare(Symbol *symbol);
        Symbol *lookupLocal(StringView name) const;
        Symbol *lookup(StringView name) const;
        const Vector<Symbol *> *lookupOverloadsLocal(StringView name) const;

    public:
        Scope *parent = nullptr;
        ASTNode *ownerNode = nullptr;

    private:
        static StringView makeOwnedKey(StringView name);

    private:
        ScopeKind m_kind;
        HashMap<StringView, Vector<Symbol *>> m_symbols;
    };
}
