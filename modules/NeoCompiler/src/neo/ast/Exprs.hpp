// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include "Base.hpp"
#include "Type.hpp"

#include <nbase/base/Assert.hpp>
#include <initializer_list>

namespace neo {

	/// Literal type enums
    enum class LiteralType : u8 {
        kUnknown,
        kU8, kU16, kU32, kU64,
        kI8, kI16, kI32, kI64,
        kF32, kF64,
        kBool
    };
    std::string_view getTypeString(LiteralType);


	/// Number literal AST node
    class NumberLiteralExpr : public ASTExpr
    {
    public:
        template <typename T>
        NumberLiteralExpr(T num);
        ~NumberLiteralExpr() = default;

    public:
        template <typename T>
        T getNumber();
	    template <typename T>
	    void setNumber(T num);

        std::string getNumberString() const;

        NE_FORCE_INLINE LiteralType getType() const {
            return m_type;
        }

    private:
        union {
            u8 u8; u16 u16; u32 u32; u64 u64;
            i8 i8; i16 i16; i32 i32; i64 i64;
            f32 f32; f64 f64;
        } m_value;
        LiteralType m_type = LiteralType::kUnknown;
    };


	/// String or text literal AST node
	class StringLiteralExpr final : public ASTExpr
	{
	public:
		StringLiteralExpr();
		StringLiteralExpr(std::string s);
		StringLiteralExpr(const std::string_view s);
		~StringLiteralExpr() override = default;

	public:

	public:
		std::string value;
	};


	/// Character literal AST node
	class CharLiteralExpr final : public ASTExpr
	{
	public:
		CharLiteralExpr();
		CharLiteralExpr(char c);
		~CharLiteralExpr() override = default;

	public:

	public:
		char value;
	};


	/// Boolean literal AST type
    class BoolLiteralExpr : public ASTExpr
    {
    public:
        BoolLiteralExpr(bool val);
        ~BoolLiteralExpr() = default;

	public:
		bool value;
    };


	/// Binary operator type's enum
    enum class BinaryOp {
        kUnknown,
        kAdd, kSub, kMul, kDiv, kMod,
        kEq, kNeq, kLt, kLe, kGt, kGe,
        kBitAnd, kBitOr, kBitXor, kShl, kShr,
        kLAnd, kLOr
    };
    std::string_view getTypeString(BinaryOp);


	/// Binary operator AST node
    class BinaryExpr : public ASTExpr
    {
    public:
        BinaryExpr(BinaryOp type, ASTExpr* l, ASTExpr* r);
        ~BinaryExpr() = default;

    public:

    public:
        ASTExpr* left;
        ASTExpr* right;
        const BinaryOp op;
    };


	/// Unary operator's enum
    enum class UnaryOp {
        kUnknown,
        kPlus,           // +a
        kMinus,          // -a
        kLogicalNot,     // !a
        kBitwiseNot,     // ~a
        kPreIncrement,   // ++a
        kPreDecrement,   // --a
        kPostIncrement,  // a++
        kPostDecrement   // a--
    };
    std::string_view toTypeString(UnaryOp);


	/// Unary operate AST node
    class UnaryExpr : public ASTExpr
    {
    public:
        UnaryExpr(UnaryOp op, ASTExpr* expr);
        ~UnaryExpr() = default;

    public:
    public:
        UnaryOp op;
        ASTExpr* operand;
    };


	/// Invoke function or call member expression AST node
    class CallExpr : public ASTExpr
    {
    public:
		CallExpr();
        CallExpr(ASTExpr* fTag, std::vector<ASTExpr*> args);
        CallExpr(ASTExpr* fTag, std::initializer_list<ASTExpr*> args);
        ~CallExpr() = default;

    public:

    public:
        ASTExpr* funcTag;
        std::vector<ASTExpr*> callArgs;
    };


	/// Accessing member expression AST node
    class MemberAccessExpr : public ASTExpr
    {
    public:
	    MemberAccessExpr();
        MemberAccessExpr(ASTExpr* object, std::string  member);
        ~MemberAccessExpr() = default;

    public:

    public:
        ASTExpr* objectExpr;
        std::string member;
        bool isUnsafeAccess = false;
    };


	/// Variable reference expression AST node
    class VariableRefExpr : public ASTExpr
    {
    public:
	    VariableRefExpr();
        VariableRefExpr(std::string  name);
        ~VariableRefExpr() = default;

    public:

    public:
        std::string variableName;
        bool moveVariable = false;
    };


	/// Type cast expression AST node
    class CastExpr : public ASTExpr
    {
    public:
        CastExpr();
		CastExpr(ASTExpr* obj, ASTTypeNode* type);
        ~CastExpr() = default;

    public:

    public:
        ASTTypeNode* castTo;
        ASTExpr* objectExpr;
    };


	/// Object create expression AST node
    class NewExpr : public ASTExpr 
    {
    public:
		NewExpr();
        NewExpr(ASTTypeNode* type, std::vector<ASTExpr*> args);
        NewExpr(ASTTypeNode* type, std::initializer_list<ASTExpr*> args);
        ~NewExpr() = default;

    public:

    public:
        ASTTypeNode* type;
        std::vector<ASTExpr*> arguments;
        bool isStackAlloc;
    };
}

#include <neo/ast/Exprs.inl>