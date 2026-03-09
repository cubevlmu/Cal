/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Exprs.hpp"

#include <utility>
#include <cstdlib>
#include "Base.hpp"
#include "Decl.hpp"
#include "neo/compiler/DebugOutput.hpp"

namespace neo
{

	static const char *s_LiteralType[] = {
		"Unknown",
		"u8", "u16", "u32", "u64",
		"i8", "i16", "i32", "i64",
		"f32", "f64", "f128", "bool"};
	StringView getTypeString(LiteralType type)
	{
		return s_LiteralType[(u8)type];
	}

	static const char *s_BinaryOp[] = {
		"Unknown",
		"Add", "Sub", "Mul", "Div", "Mod",
		"Eq", "Neq", "Lt", "Le", "Gt", "Ge",
		"BitAnd", "BitOr", "BitXor", "Shl", "Shr",
		"LAnd", "LOr",
		"Assign",
		"AddAssign", "SubAssign", "MulAssign", "DivAssign", "ModAssign",
		"ShlAssign", "ShrAssign", "AndAssign", "OrAssign", "XorAssign",
		"Comma"};

	StringView getTypeString(BinaryOp type)
	{
		return s_BinaryOp[(u8)type];
	}

	static const char *s_UnaryOp[] = {
		"Unknown",
		"Plus",			 // kPlus
		"Minus",		 // kMinus
		"LogicalNot",	 // kBang
		"BitwiseNot",	 // kTilde
		"PreIncrement",	 // kPrePlus
		"PreDecrement",	 // kPreMinus
		"PostIncrement", // kPostPlus
		"PostDecrement", // kPostMinus
		"AddressOf",	 // kAmp
		"Dereference",	 // kStar
		"Cast"			 // kCast
	};
	StringView toTypeString(UnaryOp type)
	{
		return s_UnaryOp[(u8)type];
	}

	static const char *s_PostPrefixOp[] = {
		"Unknown",
		"Inc",
		"Dec"};
	StringView toTypeString(PostPrefixOp op)
	{
		return s_PostPrefixOp[(u8)op];
	}

	String NumberLiteralExpr::getNumberString() const
	{
		switch (m_type)
		{
		case LiteralType::kU8:
			return neo::format("{}", m_value.u8);
		case LiteralType::kU16:
			return neo::format("{}", m_value.u16);
		case LiteralType::kU32:
			return neo::format("{}", m_value.u32);
		case LiteralType::kU64:
			return neo::format("{}", m_value.u64);
		case LiteralType::kI8:
			return neo::format("{}", m_value.i8);
		case LiteralType::kI16:
			return neo::format("{}", m_value.i16);
		case LiteralType::kI32:
			return neo::format("{}", m_value.i32);
		case LiteralType::kI64:
			return neo::format("{}", m_value.i64);
		case LiteralType::kF32:
			return neo::format("{}", m_value.f32);
		case LiteralType::kF64:
			return neo::format("{}", m_value.f64);
		case LiteralType::kF128:
			return neo::format("{}", m_value.f128);
		case LiteralType::kUnknown:
		case LiteralType::kBool:
			return "";
		}

		return "";
	}

	Expected<NumberLiteralExpr *> NumberLiteralExpr::parseFloatToken(const String &num)
	{
		String lower;
		lower.reserve(num.size());
		for (char c : num)
			lower += std::tolower(c);

		LiteralType ty = LiteralType::kF64; // f64 first
		String numStr = lower;

		if (numStr.ends_with("f"))
		{
			ty = LiteralType::kF32;
			numStr.pop_back();
		}
		else if (numStr.ends_with("l"))
		{
			ty = LiteralType::kF128;
			numStr.pop_back();
		}

		// remove spaces in token value
		numStr.erase(std::remove_if(numStr.begin(), numStr.end(), ::isspace), numStr.end());

		try
		{
			switch (ty)
			{
			case LiteralType::kF32:
			{
				float val = std::strtof(numStr.c_str(), nullptr);
				return new NumberLiteralExpr(val);
			}
			case LiteralType::kF64:
			{
				double val = std::strtod(numStr.c_str(), nullptr);
				return new NumberLiteralExpr(val);
			}
			case LiteralType::kF128:
			{
				long double val = std::strtold(numStr.c_str(), nullptr);
				return new NumberLiteralExpr(val);
			}
			default:
				return Result::failure("Unknown float literal type");
			}
		}
		catch (const std::exception &e)
		{
			return Result::failure(String("Invalid float literal: ") + e.what());
		}
	}

	Expected<NumberLiteralExpr *> NumberLiteralExpr::parseNumberToken(const String &txt)
	{
		String num = txt;
		bool isUnsigned = false;
		bool isLong = false;
		bool isLongLong = false;

		// check suffix
		String lower;
		for (char c : num)
			lower += std::tolower(c);
		if (lower.ends_with("ull") || lower.ends_with("llu"))
		{
			isUnsigned = true;
			isLongLong = true;
		}
		else if (lower.ends_with("ul") || lower.ends_with("lu"))
		{
			isUnsigned = true;
			isLong = true;
		}
		else if (lower.ends_with("u"))
		{
			isUnsigned = true;
		}
		else if (lower.ends_with("l"))
		{
			isLong = true;
		}

		// remove suffix
		auto pos = num.find_first_not_of("0123456789xXabcdefABCDEF");
		if (pos != String::npos)
			num = num.substr(0, pos);

		// check
		int base = 10;
		if (num.rfind("0x", 0) == 0 || num.rfind("0X", 0) == 0)
			base = 16;
		else if (num.rfind("0", 0) == 0 && num.size() > 1)
			base = 8;

		unsigned long long val = 0;
		try
		{
			val = std::strtoull(num.c_str(), nullptr, base);
		}
		catch (...)
		{
			return Result::failure("Invalid integer literal: " + num);
		}

		// check number type
		LiteralType ty = LiteralType::kUnknown;

		if (!isUnsigned && base == 10)
		{
			if (val <= kMaxI32)
				ty = LiteralType::kI32;
			else if (val <= kMaxI64)
				ty = LiteralType::kI64;
		}
		else // hex or oct or unsigned suffix
		{
			if (val <= kMaxU32)
				ty = LiteralType::kU32;
			else if (val <= kMaxU64)
				ty = LiteralType::kU64;
		}

		if (isLong && ty == LiteralType::kI32)
			ty = LiteralType::kI64;
		if (isLongLong)
			ty = LiteralType::kI64;
		if (isUnsigned && ty == LiteralType::kI32)
			ty = LiteralType::kU32;

		// fallback
		if (ty == LiteralType::kUnknown)
			ty = LiteralType::kI64;

		if (ty == LiteralType::kI32)
			return new NumberLiteralExpr((i32)val);
		if (ty == LiteralType::kI64)
			return new NumberLiteralExpr((i64)val);
		if (ty == LiteralType::kU32)
			return new NumberLiteralExpr((u32)val);
		if (ty == LiteralType::kU64)
			return new NumberLiteralExpr((u64)val);

		return Result::failure("Unknown literal type");
	}

	void NumberLiteralExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("NumberLiteral");

		out.printItem("type", getTypeString(m_type));
		out.printItem("value", getNumberString());

		out.endObject();
	}

	BoolLiteralExpr::BoolLiteralExpr(bool val)
		: ASTExpr(ExprKind::kBoolLit), value{val}
	{
	}

	void BoolLiteralExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("BoolLiteralExpr");

		out.printItem("value", value ? "true" : "false");

		out.endObject();
	}

	BinaryExpr::BinaryExpr(BinaryOp type, ASTExpr *l, ASTExpr *r)
		: ASTExpr(ExprKind::kBinary), op{type}, left{l}, right{r}
	{
	}

	void BinaryExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("BinaryExpr");

		out.printItem("op", getTypeString(op));
		out.printChild("left", left);
		out.printChild("right", right);

		out.endObject();
	}

	UnaryExpr::UnaryExpr(UnaryOp op, ASTExpr *expr)
		: ASTExpr(ExprKind::kUnary), op{op}, operand{expr}
	{
	}

	void UnaryExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("UnaryExpr");

		out.printItem("op", toTypeString(op));
		out.printChild("operand", operand);

		out.endObject();
	}

	CallExpr::CallExpr(ASTExpr *fTag, Vector<ASTExpr *> args)
		: ASTExpr(ExprKind::kFuncCall), funcTag{fTag}, callArgs{std::move(args)}
	{
	}

	CallExpr::CallExpr(ASTExpr *fTag, std::initializer_list<ASTExpr *> args)
		: ASTExpr(ExprKind::kFuncCall), funcTag{fTag}, callArgs{args}
	{
	}

	CallExpr::CallExpr()
		: ASTExpr(ExprKind::kFuncCall), funcTag{nullptr}, callArgs{}
	{
	}

	void CallExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("CallExpr");

		out.printChild("funcTag", funcTag);
		out.printChildren("arguments", callArgs);

		out.endObject();
	}

	MemberAccessExpr::MemberAccessExpr(ASTExpr *object, String member)
		: ASTExpr(ExprKind::kMemberAccess), objectExpr{object}, member{std::move(member)}
	{
	}

	MemberAccessExpr::MemberAccessExpr()
		: ASTExpr(ExprKind::kMemberAccess), objectExpr{nullptr}, member{}
	{
	}

	void MemberAccessExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("MemberAccessExpr");

		out.printChild("object", objectExpr);
		out.printItem("member", member);

		out.endObject();
	}

	VariableRefExpr::VariableRefExpr(String name)
		: ASTExpr(ExprKind::kVar), variableName{std::move(name)}
	{
	}

	VariableRefExpr::VariableRefExpr()
		: ASTExpr(ExprKind::kVar), variableName{}
	{
	}

	void VariableRefExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("VariableRefExpr");

		out.printItem("name", variableName);
		out.printItem("moveVariable", moveVariable ? "true" : "false");

		out.endObject();
	}

	CastExpr::CastExpr(ASTExpr *obj, ASTTypeNode *type)
		: ASTExpr(ExprKind::kCast), castTo{type}, objectExpr{obj}
	{
	}

	CastExpr::CastExpr()
		: ASTExpr(ExprKind::kCast), castTo{nullptr}, objectExpr{nullptr}
	{
	}

	void CastExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("CastExpr");

		out.printChild("castTo", castTo);
		out.printChild("object", objectExpr);

		out.endObject();
	}

	NewExpr::NewExpr()
		: ASTExpr(ExprKind::kNew)
	{
	}

	NewExpr::NewExpr(ASTTypeNode *type, Vector<ASTExpr *> args)
		: ASTExpr(ExprKind::kNew), type{type}, arguments{std::move(args)}
	{
	}

	NewExpr::NewExpr(ASTTypeNode *type, std::initializer_list<ASTExpr *> args)
		: ASTExpr(ExprKind::kNew), type{type}, arguments{args}
	{
	}

	void NewExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("NewExpr");

		out.printChild("type", type);
		out.printItem("isStackAlloc", isStackAlloc ? "true" : "false");

		if (arguments.empty())
		{
			out.printItem("arguments", "<empty>");
		}
		else
		{
			out.printChildren("arguments", arguments);
		}

		out.endObject();
	}

	StringLiteralExpr::StringLiteralExpr()
		: ASTExpr(ExprKind::kStringLit), value{}
	{
	}

	StringLiteralExpr::StringLiteralExpr(String s)
		: ASTExpr(ExprKind::kStringLit), value{std::move(s)}
	{
	}

	StringLiteralExpr::StringLiteralExpr(const StringView s)
		: ASTExpr(ExprKind::kStringLit), value{s}
	{
	}

	void StringLiteralExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("StringLiteral");

		out.printItem("value", value);

		out.endObject();
	}

	CharLiteralExpr::CharLiteralExpr()
		: ASTExpr(ExprKind::kCharLit), value{}
	{
	}

	CharLiteralExpr::CharLiteralExpr(char c)
		: ASTExpr(ExprKind::kCharLit), value{c}
	{
	}

	void CharLiteralExpr::debugPrint(NDebugOutput &out)
	{
		out.beginObject("CharLiteral");

		out.printItem("value", value);

		out.endObject();
	}

	LambdaFuncExpr::LambdaFuncExpr()
		: ASTExpr(ExprKind::kLambda), function{}
	{
	}

	LambdaFuncExpr::LambdaFuncExpr(struct FuncDecl *func)
		: ASTExpr(ExprKind::kLambda), function{func}
	{
	}

	void LambdaFuncExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("LambdaFuncExpr");

		out.printChild("body", function);

		out.endObject();
	}

	ArrayLiteralExpr::ArrayLiteralExpr()
		: ASTExpr(ExprKind::kArrayLit), elements{}
	{
	}

	ArrayLiteralExpr::ArrayLiteralExpr(Vector<ASTExpr *> v)
		: ASTExpr(ExprKind::kArrayLit), elements{std::move(v)}
	{
	}

	ArrayLiteralExpr::~ArrayLiteralExpr()
	{
		// TODO clean up
	}

	void ArrayLiteralExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("ArrayLiteralExpr");

		if (elements.empty())
		{
			out.printItem("elements", "<empty>");
		}
		else
		{
			out.printChildren("elements", elements);
		}

		out.endObject();
	}

	SubscriptExpr::SubscriptExpr()
		: ASTExpr(ExprKind::kSubscript), indexExpr{nullptr}
	{
	}

	SubscriptExpr::SubscriptExpr(ASTExpr *index)
		: ASTExpr(ExprKind::kSubscript), indexExpr{index}
	{
	}

	void SubscriptExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("SubscriptExpr");

		out.printChild("index", indexExpr);

		out.endObject();
	}

	PostfixExpr::PostfixExpr()
		: ASTExpr(ExprKind::kPostfix), op{kUnknown}, postfixExpr{nullptr}
	{
	}

	PostfixExpr::PostfixExpr(PostPrefixOp op, ASTExpr *expr)
		: ASTExpr(ExprKind::kPostfix), op{op}, postfixExpr{expr}
	{
	}

	void PostfixExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("PostfixExpr");

		out.printItem("op", toTypeString(op));
		out.printChild("expr", postfixExpr);

		out.endObject();
	}

	ConditionalExpr::ConditionalExpr()
		: ASTExpr(ExprKind::kCond), condExpr{nullptr}, trueExpr{nullptr}, falseExpr{nullptr}
	{
	}

	ConditionalExpr::ConditionalExpr(ASTExpr *cond, ASTExpr *trueExpr, ASTExpr *falseExpr)
		: ASTExpr(ExprKind::kCond), condExpr{cond}, trueExpr{trueExpr}, falseExpr{falseExpr}
	{
	}

	void ConditionalExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("ConditionalExpr");

		out.printChild("cond", condExpr);
		out.printChild("trueBranch", trueExpr);
		out.printChild("falseBranch", falseExpr);

		out.endObject();
	}

	CommaExpr::CommaExpr()
		: ASTExpr(ExprKind::kComma), leftExpr{nullptr}, rightExpr{nullptr}
	{
	}

	CommaExpr::CommaExpr(ASTExpr *left, ASTExpr *right)
		: ASTExpr(ExprKind::kComma), leftExpr{left}, rightExpr{right}
	{
	}

	void CommaExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("CommaExpr");

		out.printChild("left", leftExpr);
		out.printChild("right", rightExpr);

		out.endObject();
	}

	void ASTIdent::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("IdentExpr");

		out.printItem("name", name);

		out.endObject();
	}

	void NullExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("NullExpr");
		out.endObject();
	}

	void ThisExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("ThisExpr");
		out.endObject();
	}

	void SuperExpr::debugPrint(NDebugOutput &out)
	{
		ASTExpr::debugPrint(out);
		out.beginObject("SuperExpr");
		out.endObject();
	}
}
