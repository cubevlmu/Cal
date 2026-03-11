/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Scopes.hpp"

#include "Symbols.hpp"

namespace neo
{
    StringView getTypeString(ScopeKind kind)
    {
        switch (kind)
        {
        case ScopeKind::kUnknown:
            return "kUnknown";
        case ScopeKind::kGlobal:
            return "kGlobal";
        case ScopeKind::kModule:
            return "kModule";
        case ScopeKind::kType:
            return "kType";
        case ScopeKind::kFunction:
            return "kFunction";
        case ScopeKind::kLocal:
            return "kLocal";
        case ScopeKind::kLoop:
            return "kLoop";
        case ScopeKind::kGeneric:
            return "kGeneric";
        default:
            return "Unknown ScopeKind";
        }
    }

    Scope::Scope(ScopeKind kind, ASTNode *owner, Scope *scopeParent)
        : parent{scopeParent}, ownerNode{owner}, m_kind{kind}
    {
    }

    bool Scope::declare(Symbol *symbol)
    {
        if (symbol == nullptr || symbol->name.empty())
        {
            return false;
        }

        const StringView key{symbol->name.c_str(), static_cast<psize>(symbol->name.length())};
        auto &bucket = m_symbols[key];
        bucket.push_back(symbol);
        symbol->ownerScope = this;
        return true;
    }

    Symbol *Scope::lookupLocal(StringView name) const
    {
        const auto *bucket = lookupOverloadsLocal(name);
        if (bucket == nullptr || bucket->empty())
        {
            return nullptr;
        }
        return bucket->front();
    }

    Symbol *Scope::lookup(StringView name) const
    {
        for (auto *scope = this; scope != nullptr; scope = scope->parent)
        {
            if (auto *found = scope->lookupLocal(name); found != nullptr)
            {
                return found;
            }
        }
        return nullptr;
    }

    const Vector<Symbol *> *Scope::lookupOverloadsLocal(StringView name) const
    {
        const auto key = makeOwnedKey(name);
        auto it = m_symbols.find(key);
        if (it == m_symbols.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    StringView Scope::makeOwnedKey(StringView name)
    {
        return name;
    }
}
