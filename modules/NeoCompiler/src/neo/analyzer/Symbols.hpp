/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/common.hpp>

#include "neo/ast/Base.hpp"

namespace neo
{
    class Scope;
    class SemanticType;

    enum class SymbolKind
    {
        kUnknown,
        kError,
        kModule,
        kFunction,
        kVariable,
        kParameter,
        kField,
        kProperty,
        kClass,
        kStruct,
        kInterface,
        kEnum,
        kEnumItem,
        kGenericParam,
    };
    StringView getTypeString(SymbolKind kind);

    class Symbol
    {
    public:
        Symbol(SymbolKind kind, StringView name, ASTDecl *decl = nullptr);
        virtual ~Symbol() = default;

    public:
        SymbolKind getKind() const
        {
            return m_kind;
        }

    public:
        String name;
        Scope *ownerScope = nullptr;
        ASTDecl *decl = nullptr;
        ASTModifier modifier;

    private:
        SymbolKind m_kind;
    };

    class ErrorSymbol final : public Symbol
    {
    public:
        explicit ErrorSymbol(StringView name);
    };

    class ModuleSymbol final : public Symbol
    {
    public:
        ModuleSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        Scope *moduleScope = nullptr;
    };

    class TypeSymbol : public Symbol
    {
    public:
        TypeSymbol(SymbolKind kind, StringView name, ASTDecl *decl = nullptr);

    public:
        Scope *memberScope = nullptr;
        Vector<class GenericParamSymbol *> genericParams;
    };

    class ClassSymbol final : public TypeSymbol
    {
    public:
        ClassSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        TypeSymbol *baseClass = nullptr;
        Vector<TypeSymbol *> interfaces;
    };

    class StructSymbol final : public TypeSymbol
    {
    public:
        StructSymbol(StringView name, ASTDecl *decl = nullptr);
    };

    class InterfaceSymbol final : public TypeSymbol
    {
    public:
        InterfaceSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        Vector<TypeSymbol *> baseInterfaces;
    };

    class EnumSymbol final : public TypeSymbol
    {
    public:
        EnumSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        SemanticType *underlyingType = nullptr;
    };

    class FunctionSymbol final : public Symbol
    {
    public:
        FunctionSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        Scope *functionScope = nullptr;
        Vector<class VariableSymbol *> params;
        Vector<class GenericParamSymbol *> genericParams;
        SemanticType *returnType = nullptr;
        TypeSymbol *ownerType = nullptr;
        bool isMethod = false;
        bool isCtor = false;
        bool isDtor = false;
        bool isVirtual = false;
        bool isOverride = false;
        bool isImpl = false;
    };

    class VariableSymbol : public Symbol
    {
    public:
        VariableSymbol(SymbolKind kind, StringView name, ASTDecl *decl = nullptr);

    public:
        SemanticType *type = nullptr;
        bool isMutable = false;
        bool isLocal = false;
        bool isGlobal = false;
    };

    class FieldSymbol final : public VariableSymbol
    {
    public:
        FieldSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        TypeSymbol *ownerType = nullptr;
    };

    class PropertySymbol final : public Symbol
    {
    public:
        PropertySymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        SemanticType *type = nullptr;
        TypeSymbol *ownerType = nullptr;
    };

    class EnumItemSymbol final : public Symbol
    {
    public:
        EnumItemSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        EnumSymbol *ownerEnum = nullptr;
    };

    class GenericParamSymbol final : public Symbol
    {
    public:
        GenericParamSymbol(StringView name, ASTDecl *decl = nullptr);

    public:
        Vector<SemanticType *> constraints;
        i32 index = -1;
    };
}
