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
    class ASTTypeNode;

    enum class SemanticTypeKind
    {
        kUnknown,
        kError,
        kVoid,
        kBool,
        kInt,
        kFloat,
        kString,
        kNull,
        kNamed,
        kClass,
        kStruct,
        kInterface,
        kEnum,
        kFunction,
        kGenericParam,
        kGenericInstance
    };
    StringView getTypeString(SemanticTypeKind kind);

    enum class TypeQualifier : u32
    {
        kNone = 0,
        kConst = 1 << 0,
    };

    class SemanticType
    {
    public:
        explicit SemanticType(SemanticTypeKind kind);
        virtual ~SemanticType() = default;

    public:
        SemanticTypeKind getKind() const
        {
            return m_kind;
        }

    public:
        TypeQualifier qualifiers = TypeQualifier::kNone;
        SemanticType *canonicalType = nullptr;

    private:
        SemanticTypeKind m_kind;
    };

    class ErrorType final : public SemanticType
    {
    public:
        ErrorType();
    };

    class BuiltinType final : public SemanticType
    {
    public:
        BuiltinType(SemanticTypeKind kind, StringView name);

    public:
        String name;
    };

    class NamedType : public SemanticType
    {
    public:
        NamedType(SemanticTypeKind kind, StringView name);

    public:
        String name;
        ASTTypeNode *syntax = nullptr;
    };

    class FunctionType final : public SemanticType
    {
    public:
        FunctionType();

    public:
        Vector<SemanticType *> paramTypes;
        SemanticType *returnType = nullptr;
        bool isMethod = false;
    };

    class GenericParamType final : public SemanticType
    {
    public:
        explicit GenericParamType(StringView name);

    public:
        String name;
    };

    class GenericInstanceType final : public SemanticType
    {
    public:
        explicit GenericInstanceType(SemanticType *primary);

    public:
        SemanticType *primaryType = nullptr;
        Vector<SemanticType *> arguments;
    };

    constexpr TypeQualifier operator|(TypeQualifier lhs, TypeQualifier rhs)
    {
        return static_cast<TypeQualifier>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
    }
}
