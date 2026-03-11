/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/common.hpp>

#include "neo/diagnose/SourceLoc.hpp"
#include <nbase/memory/Memory.hpp>

namespace neo
{
    enum class TokenType : u8
    {
        kUnknown, //
        kEOF,     // end of file

        kIdentifier, // identifier
        kTypeId,     // type identifier

        kImport, // import
        kExport, // export
        kInline, // inline
        kFinal,  // final
        kModule, // module
        kExtern, // extern

        kFun, // function
        kIf,
        kElse, //
        kFor,
        kWhile, //

        kClass,     // class
        kStruct,    // struct
        kInterface, // interface
        kAttribute, // annotation
        kEnum,      // enum

        kCtor,  // constructor
        kDtor,  // destructor
        kField, // field

        kPrivate,
        kProtected,
        kStatic,
        kVirtual,  // virtual function
        kInternal, // internal
        kOverride, // override
        kImpl,     // implements / implement

        kVar,   // variable
        kVal,   // non-changed variable
        kConst, // constant

        kAdd, // +
        kSub, // -
        kMul, // *
        kDiv, // /
        kMod, // %

        kIsEqual, // ==

        kLBracket,
        kRBracket, // [ ]
        kLBraces,
        kRBraces, // { }
        kLParen,
        kRParen, // ( )

        kCharLit,   // char
        kStringLit, // string
        kIntLit,    // number
        kHexLit,    // hex number
        kFloatLit,  // float number

        kTrue,
        kFalse, // bool value
        kNull,  // null value

        kFuncCall, // reserved
        kReturn,   // return

        kComma,       // ,
        kDot,         // .
        kColon,       // :
        kDoubleColon, // ::
        kSemicolon,   // ;
        kQuestion,    // ?
        kArrow,       // ->

        kLAnd, // &&
        kLOr,  // ||
        kLNot, // !

        kEq,  // ==
        kNeq, // !=
        kLt,  // <
        kGt,  // >
        kLe,  // <=
        kGe,  // >=

        kInc,    // ++
        kDec,    // --
        kShl,    // <<
        kShr,    // >>
        kBitAnd, // &
        kBitOr,  // |
        kBitXor, // ^
        kBitNot, // ~

        kAssign,    // =
        kAddAssign, // +=
        kSubAssign, // -=
        kMulAssign, // *=
        kDivAssign, // /=
        kModAssign, // %=
        kShlAssign, // <<=
        kShrAssign, // >>=
        kAndAssign, // &=
        kOrAssign,  // |=
        kXorAssign, // ^=

        kTry,
        kCatch,
        kFinally,
        kThrow,
        kBreak,
        kContinue,
        kNew,
        kCast,
        kThis,
        kSuper
    };

    struct NToken final
    {
        TokenType type;
        String value;
        psize line;
        psize cursor;

        static StringView typeString(TokenType);
        static TokenType checkIdentifier(const StringView &str);
        String toString() const;
        StringView typeString() const;

        bool operator==(const NToken &other) const
        {
            return type == other.type && value == other.value;
        }
        bool operator!=(const NToken &other) const
        {
            return !(*this == other);
        }

        SourceLoc location(NSourceFile *file = nullptr) const
        {
            return SourceLoc{line, cursor, file};
        }

        static NToken Invalid;
    };
}
