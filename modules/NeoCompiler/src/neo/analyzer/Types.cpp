/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Types.hpp"

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

    StringView getTypeString(SemanticTypeKind kind)
    {
        switch (kind)
        {
        case SemanticTypeKind::kUnknown:
            return "kUnknown";
        case SemanticTypeKind::kError:
            return "kError";
        case SemanticTypeKind::kVoid:
            return "kVoid";
        case SemanticTypeKind::kBool:
            return "kBool";
        case SemanticTypeKind::kInt:
            return "kInt";
        case SemanticTypeKind::kFloat:
            return "kFloat";
        case SemanticTypeKind::kString:
            return "kString";
        case SemanticTypeKind::kNull:
            return "kNull";
        case SemanticTypeKind::kNamed:
            return "kNamed";
        case SemanticTypeKind::kClass:
            return "kClass";
        case SemanticTypeKind::kStruct:
            return "kStruct";
        case SemanticTypeKind::kInterface:
            return "kInterface";
        case SemanticTypeKind::kEnum:
            return "kEnum";
        case SemanticTypeKind::kFunction:
            return "kFunction";
        case SemanticTypeKind::kGenericParam:
            return "kGenericParam";
        case SemanticTypeKind::kGenericInstance:
            return "kGenericInstance";
        default:
            return "Unknown SemanticTypeKind";
        }
    }

    SemanticType::SemanticType(SemanticTypeKind kind)
        : canonicalType{this}, m_kind{kind}
    {
    }

    ErrorType::ErrorType()
        : SemanticType(SemanticTypeKind::kError)
    {
    }

    BuiltinType::BuiltinType(SemanticTypeKind kind, StringView typeName)
        : SemanticType(kind), name{makeOwnedString(typeName)}
    {
    }

    NamedType::NamedType(SemanticTypeKind kind, StringView typeName)
        : SemanticType(kind), name{makeOwnedString(typeName)}
    {
    }

    FunctionType::FunctionType()
        : SemanticType(SemanticTypeKind::kFunction)
    {
    }

    GenericParamType::GenericParamType(StringView paramName)
        : SemanticType(SemanticTypeKind::kGenericParam), name{makeOwnedString(paramName)}
    {
    }

    GenericInstanceType::GenericInstanceType(SemanticType *primary)
        : SemanticType(SemanticTypeKind::kGenericInstance), primaryType{primary}
    {
    }
}
