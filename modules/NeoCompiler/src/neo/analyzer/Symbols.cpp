/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Symbols.hpp"

namespace neo
{
    namespace
    {
        String makeOwnedString(StringView view)
        {
            if (view.empty())
            {
                return {};
            }
            return {view.data(), view.size()};
        }
    }

    StringView getTypeString(SymbolKind kind)
    {
        switch (kind)
        {
        case SymbolKind::kUnknown:
            return "kUnknown";
        case SymbolKind::kError:
            return "kError";
        case SymbolKind::kModule:
            return "kModule";
        case SymbolKind::kFunction:
            return "kFunction";
        case SymbolKind::kVariable:
            return "kVariable";
        case SymbolKind::kParameter:
            return "kParameter";
        case SymbolKind::kField:
            return "kField";
        case SymbolKind::kProperty:
            return "kProperty";
        case SymbolKind::kClass:
            return "kClass";
        case SymbolKind::kStruct:
            return "kStruct";
        case SymbolKind::kInterface:
            return "kInterface";
        case SymbolKind::kEnum:
            return "kEnum";
        case SymbolKind::kEnumItem:
            return "kEnumItem";
        case SymbolKind::kGenericParam:
            return "kGenericParam";
        default:
            return "Unknown SymbolKind";
        }
    }

    Symbol::Symbol(SymbolKind kind, StringView symbolName, ASTDecl *symbolDecl)
        : name{makeOwnedString(symbolName)}, decl{symbolDecl}, m_kind{kind}
    {
        if (decl != nullptr)
        {
            modifier = decl->modifier;
        }
    }

    ErrorSymbol::ErrorSymbol(StringView symbolName)
        : Symbol(SymbolKind::kError, symbolName, nullptr)
    {
    }

    ModuleSymbol::ModuleSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(SymbolKind::kModule, symbolName, symbolDecl)
    {
    }

    TypeSymbol::TypeSymbol(SymbolKind kind, StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(kind, symbolName, symbolDecl)
    {
    }

    ClassSymbol::ClassSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : TypeSymbol(SymbolKind::kClass, symbolName, symbolDecl)
    {
    }

    StructSymbol::StructSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : TypeSymbol(SymbolKind::kStruct, symbolName, symbolDecl)
    {
    }

    InterfaceSymbol::InterfaceSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : TypeSymbol(SymbolKind::kInterface, symbolName, symbolDecl)
    {
    }

    EnumSymbol::EnumSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : TypeSymbol(SymbolKind::kEnum, symbolName, symbolDecl)
    {
    }

    FunctionSymbol::FunctionSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(SymbolKind::kFunction, symbolName, symbolDecl)
    {
    }

    VariableSymbol::VariableSymbol(SymbolKind kind, StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(kind, symbolName, symbolDecl)
    {
    }

    FieldSymbol::FieldSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : VariableSymbol(SymbolKind::kField, symbolName, symbolDecl)
    {
    }

    PropertySymbol::PropertySymbol(StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(SymbolKind::kProperty, symbolName, symbolDecl)
    {
    }

    EnumItemSymbol::EnumItemSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(SymbolKind::kEnumItem, symbolName, symbolDecl)
    {
    }

    GenericParamSymbol::GenericParamSymbol(StringView symbolName, ASTDecl *symbolDecl)
        : Symbol(SymbolKind::kGenericParam, symbolName, symbolDecl)
    {
    }
}
