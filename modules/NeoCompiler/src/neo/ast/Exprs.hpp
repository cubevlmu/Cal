/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "Base.hpp"
#include "Type.hpp"
#include "neo/diagnose/Diagnostic.hpp"

#include <nbase/base/Assert.hpp>
#include <initializer_list>

namespace neo
{

	/// AST identifier for lvalue and rvalue
	class ASTIdent : public ASTExpr
	{
	public:
		ASTIdent() : ASTExpr(ExprKind::kIdent) {}
		ASTIdent(String name)
			: ASTExpr(ExprKind::kIdent), name{std::move(name)}
		{
		}

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		String name;
	};

	/// Literal type enums
	enum class LiteralType : u8
	{
		kUnknown,
		kU8,
		kU16,
		kU32,
		kU64,
		kI8,
		kI16,
		kI32,
		kI64,
		kF32,
		kF64,
		kF128,
		kBool
	};
	StringView getTypeString(LiteralType);

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

		String getNumberString() const;

		NE_FORCE_INLINE LiteralType getType() const
		{
			return m_type;
		}

		static Expected<NumberLiteralExpr *> parseNumberToken(const String &num);
		static Expected<NumberLiteralExpr *> parseFloatToken(const String &num);

	public:
		void debugPrint(NDebugOutput &out) override;

	private:
		union
		{
			u8 u8;
			u16 u16;
			u32 u32;
			u64 u64;
			i8 i8;
			i16 i16;
			i32 i32;
			i64 i64;
			f32 f32;
			f64 f64;
			f128 f128;
		} m_value;
		LiteralType m_type = LiteralType::kUnknown;
	};

	/// String or text literal AST node
	class StringLiteralExpr final : public ASTExpr
	{
	public:
		StringLiteralExpr();
		StringLiteralExpr(String s);
		StringLiteralExpr(const StringView s);
		~StringLiteralExpr() override = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		String value;
	};

	/// Character literal AST node
	class CharLiteralExpr final : public ASTExpr
	{
	public:
		CharLiteralExpr();
		CharLiteralExpr(char c);
		~CharLiteralExpr() override = default;

	public:
		void debugPrint(NDebugOutput &out) override;

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
		void debugPrint(NDebugOutput &out) override;

	public:
		bool value;
	};

	/// Binary operator type's enum
	enum class BinaryOp
	{
		kUnknown,

		// Arithmetic
		kAdd, // +
		kSub, // -
		kMul, // *
		kDiv, // /
		kMod, // %

		// Comparison
		kEq,  // ==
		kNeq, // !=
		kLt,  // <
		kLe,  // <=
		kGt,  // >
		kGe,  // >=

		// Bitwise
		kBitAnd, // &
		kBitOr,	 // |
		kBitXor, // ^
		kShl,	 // <<
		kShr,	 // >>

		// Logical
		kLAnd, // &&
		kLOr,  // ||

		// Assignment
		kAssign,	// =
		kAddAssign, // +=
		kSubAssign, // -=
		kMulAssign, // *=
		kDivAssign, // /=
		kModAssign, // %=
		kShlAssign, // <<=
		kShrAssign, // >>=
		kAndAssign, // &=
		kOrAssign,	// |=
		kXorAssign, // ^=

		// Other binary-like constructs
		kComma, // , (if you support comma expressions)
	};
	StringView getTypeString(BinaryOp);

	/// Binary operator AST node
	class BinaryExpr : public ASTExpr
	{
	public:
		BinaryExpr(BinaryOp type, ASTExpr *l, ASTExpr *r);
		~BinaryExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTExpr *left;
		ASTExpr *right;
		const BinaryOp op;
	};

	/// Unary operator's enum
	enum class UnaryOp
	{
		kUnknown,
		kPlus,	   // +a
		kMinus,	   // -a
		kBang,	   // !a
		kTilde,	   // ~a
		kPrePlus,  // ++a
		kPreMinus, // --a
		kUnused1,  //
		kUnused2,  //
		kAmp,	   // &a
		kStar,	   // *a
		kCast,	   // (T)a
	};
	StringView toTypeString(UnaryOp);

	/// Unary operate AST node
	class UnaryExpr : public ASTExpr
	{
	public:
		UnaryExpr(UnaryOp op, ASTExpr *expr);
		~UnaryExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		UnaryOp op;
		ASTExpr *operand;
	};

	/// Invoke function or call member expression AST node
	class CallExpr : public ASTExpr
	{
	public:
		CallExpr();
		CallExpr(ASTExpr *fTag, Vector<ASTExpr *> args);
		CallExpr(ASTExpr *fTag, std::initializer_list<ASTExpr *> args);
		~CallExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTExpr *funcTag;
		Vector<ASTExpr *> callArgs;
	};

	/// Accessing member expression AST node
	class MemberAccessExpr : public ASTExpr
	{
	public:
		MemberAccessExpr();
		MemberAccessExpr(ASTExpr *object, String member);
		~MemberAccessExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTExpr *objectExpr;
		String member;
		bool isUnsafeAccess = false;
	};

	/// Array's initial value expression
	class ArrayLiteralExpr : public ASTExpr
	{
	public:
		ArrayLiteralExpr();
		ArrayLiteralExpr(Vector<ASTExpr *> v);
		~ArrayLiteralExpr();

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		Vector<ASTExpr *> elements;
	};

	/// Variable reference expression AST node
	class VariableRefExpr : public ASTExpr
	{
	public:
		VariableRefExpr();
		VariableRefExpr(String name);
		~VariableRefExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		String variableName;
		bool moveVariable = false;
	};

	/// Type cast expression AST node
	class CastExpr : public ASTExpr
	{
	public:
		CastExpr();
		CastExpr(ASTExpr *obj, ASTTypeNode *type);
		~CastExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTTypeNode *castTo;
		ASTExpr *objectExpr;
	};

	/// Object create expression AST node
	class NewExpr : public ASTExpr
	{
	public:
		NewExpr();
		NewExpr(ASTTypeNode *type, Vector<ASTExpr *> args);
		NewExpr(ASTTypeNode *type, std::initializer_list<ASTExpr *> args);
		~NewExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTTypeNode *type;
		Vector<ASTExpr *> arguments;
		bool isStackAlloc;
	};

	/// Null value expression
	class NullExpr : public ASTExpr, public SingletonExpr<NullExpr>
	{
		friend class SingletonExpr;

	public:
		NullExpr() : ASTExpr(ExprKind::kNull) {}
		void debugPrint(NDebugOutput &out) override;
	};

	/// This expression
	class ThisExpr : public ASTExpr, public SingletonExpr<ThisExpr>
	{
		friend class SingletonExpr;

	public:
		ThisExpr() : ASTExpr(ExprKind::kThis) {}
		void debugPrint(NDebugOutput &out) override;
	};

	/// This expression
	class SuperExpr : public ASTExpr, public SingletonExpr<SuperExpr>
	{
		friend class SingletonExpr;

	public:
		SuperExpr() : ASTExpr(ExprKind::kSuper) {}
		void debugPrint(NDebugOutput &out) override;
	};

	/// Lambda expression support
	class LambdaFuncExpr : public ASTExpr
	{
	public:
		LambdaFuncExpr();
		LambdaFuncExpr(class FuncDecl *func);
		~LambdaFuncExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		class FuncDecl *function;
	};

	/// Index access expression support
	class SubscriptExpr : public ASTExpr
	{
	public:
		SubscriptExpr();
		SubscriptExpr(ASTExpr *index);
		~SubscriptExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTExpr *indexExpr;
	};

	/// PostPrefix operator's enum
	enum class PostPrefixOp : u8
	{
		kUnknown,
		kInc, // a++
		kDec, // a--
	};
	StringView toTypeString(PostPrefixOp);

	/// PostPrefix operator expression
	class PostfixExpr : public ASTExpr
	{
	public:
		PostfixExpr();
		PostfixExpr(PostPrefixOp op, ASTExpr *expr);
		~PostfixExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		PostPrefixOp op;
		ASTExpr *postfixExpr;
	};

	/// Conditional expression support
	class ConditionalExpr : public ASTExpr
	{
	public:
		ConditionalExpr();
		ConditionalExpr(ASTExpr *cond, ASTExpr *trueExpr, ASTExpr *falseExpr);
		~ConditionalExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTExpr *condExpr;
		ASTExpr *trueExpr;
		ASTExpr *falseExpr;
	};

	class CommaExpr : public ASTExpr
	{
	public:
		CommaExpr();
		CommaExpr(ASTExpr *left, ASTExpr *right);
		~CommaExpr() = default;

	public:
		void debugPrint(NDebugOutput &out) override;

	public:
		ASTExpr *leftExpr;
		ASTExpr *rightExpr;
	};
}

#include <neo/ast/Exprs.inl>
