// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Tokens.hpp"

#include <map>

namespace neo {

    static const char *s_typeStrings[] = {
            "Unknown",       // kUnknown
            "EOF",           // kEOF

            "Identifier",    // kIdentifier
            "TypeId",        // kTypeId

            "Import",        // kImport
            "Export",        // kExport
            "Inline",        // kInline
            "Final",         // kFinal
            "Module",        // kModule
            "Extern",        // kExtern

            "Fun",           // kFun
            "If", "Else",    // kIf, kElse
            "For", "While",  // kFor, kWhile

            "Class",         // kClass
            "Struct",        // kStruct
            "Interface",     // kInterface
            "Attribute",     // kAttribute
            "Enum",          // kEnum

            "Ctor",          // kCtor
            "Dtor",          // kDtor
            "Field",         // kField

            "Private", "Protected", "Static", // kPrivate, kProtected, kStatic
            "Virtual",       // kVirtual
            "Internal",      // kInternal
            "Override",      // kOverride
            "Impl",          // kImpl

            "Var",           // kVar
            "Val",           // kVal
            "Const",         // kConst

            "Add",           // kAdd
            "Sub",           // kSub
            "Mul",           // kMul
            "Div",           // kDiv
            "Mod",           // kMod

            "IsEqual",       // kIsEqual

            "LBracket", "RBracket", // kLBracket, kRBracket
            "LBraces", "RBraces",   // kLBraces, kRBraces
            "LParen", "RParen",     // kLParen, kRParen

            "CharLit",     // kCharLit
            "StringLit",   // kStringLit
            "IntLit",      // kIntLit
            "HexLit",      // kHexLit
            "FloatLit",    // kFloatLit

            "True", "False", "Null", // kTrue, kFalse, kNull

            "FuncCall",    // kFuncCall
            "Return",      // kReturn

            "Comma",       // kComma
            "Dot",         // kDot
            "Colon",       // kColon
            "DoubleColon", // kDoubleColon
            "Semicolon",   // kSemicolon
            "Question",    // kQuestion
            "Arrow",       // kArrow

            "LAnd",        // kLAnd
            "LOr",         // kLOr
            "LNot",        // kLNot

            "Eq",          // kEq
            "Neq",         // kNeq
            "Lt",          // kLt
            "Gt",          // kGt
            "Le",          // kLe
            "Ge",          // kGe

            "Inc",         // kInc
            "Dec",         // kDec
            "Shl",         // kShl
            "Shr",         // kShr
            "BitAnd",      // kBitAnd
            "BitOr",       // kBitOr
            "BitXor",      // kBitXor
            "BitNot",      // kBitNot

            "Assign",      // kAssign
            "AddAssign",   // kAddAssign
            "SubAssign",   // kSubAssign
            "MulAssign",   // kMulAssign
            "DivAssign",   // kDivAssign
            "ModAssign",   // kModAssign
            "ShlAssign",   // kShlAssign
            "ShrAssign",   // kShrAssign
            "AndAssign",   // kAndAssign
            "OrAssign",    // kOrAssign
            "XorAssign",   // kXorAssign

            "Try", "Catch", "Finally", "Throw", // kTry, kCatch, kFinally, kThrow
            "Break", "Continue",               // kBreak, kContinue
            "New",                               // kNew
			"Cast",        // cast
			"This",        // this
			"Super"        // super
    };
    static_assert(sizeof(s_typeStrings) / sizeof(s_typeStrings[0]) == static_cast<size_t>(TokenType::kSuper) + 1,
                  "s_typeStrings array size does not match TokenType enum count");


    static const std::map<std::string_view, TokenType> s_idTypes{
            {"var",       TokenType::kVar},
            {"val",       TokenType::kVal},
            {"fun",       TokenType::kFun},
            {"class",     TokenType::kClass},
            {"struct",    TokenType::kStruct},
            {"import",    TokenType::kImport},
            {"module",    TokenType::kModule},
            {"export",    TokenType::kExport},
            {"const",     TokenType::kConst},
            {"ctor",      TokenType::kCtor},
            {"dtor",      TokenType::kDtor},
            {"private",   TokenType::kPrivate},
            {"protected", TokenType::kProtected},
            {"internal",  TokenType::kInternal},
            {"impl",      TokenType::kImpl},
            {"if",        TokenType::kIf},
            {"else",      TokenType::kElse},
            {"inline",    TokenType::kInline},
            {"while",     TokenType::kWhile},
            {"for",       TokenType::kFor},
            {"interface", TokenType::kInterface},
            {"virtual",   TokenType::kVirtual},
            {"enum",      TokenType::kEnum},
            {"false",     TokenType::kFalse},
            {"true",      TokenType::kTrue},
            {"field",     TokenType::kField},
            {"try",       TokenType::kTry},
            {"catch",     TokenType::kCatch},
            {"finally",   TokenType::kFinally},
            {"throw",     TokenType::kThrow},
            {"continue",  TokenType::kContinue},
            {"break",     TokenType::kBreak},
            {"new",       TokenType::kNew},
            {"extern",    TokenType::kExtern},
            {"final",     TokenType::kFinal},
            {"super",     TokenType::kSuper},
            {"this",      TokenType::kThis},
            {"cast",      TokenType::kCast}
    };


    TokenType NToken::checkIdentifier(const std::string_view &c) {
        std::string_view rc = c.data();
        if (s_idTypes.find(rc) != s_idTypes.end()) {
            return s_idTypes.at(rc);
        }
        return TokenType::kUnknown;
    }


    std::string_view NToken::typeString(TokenType tk) {
        int idx = (int) tk;
        if (idx < 0 || idx > 86) {
            return "";
        }
        return s_typeStrings[idx];
    }


    std::string NToken::toString() const {
        return "{ " + std::string(s_typeStrings[(int) type]) + " : " + value + " }";
    }


    std::string_view NToken::typeString() const {
        return typeString(type);
    }


    NToken NToken::Invalid{TokenType::kUnknown, ""};
}