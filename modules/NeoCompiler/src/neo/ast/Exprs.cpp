// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Exprs.hpp"

#include <utility>
#include "Base.hpp"

namespace neo
{

	static const char* s_LiteralType[] = {
		"Unknown",
		"u8", "u16", "u32", "u64",
		"i8", "i16", "i32", "i64",
		"f32", "f64", "f128", "bool"
	};
	std::string_view getTypeString(LiteralType type)
	{
		return s_LiteralType[(u8)type];
	}

	static const char* s_BinaryOp[] = {
		"Unknown",
		"Add", "Sub", "Mul", "Div", "Mod",
		"Eq", "Neq", "Lt", "Le", "Gt", "Ge",
		"BitAnd", "BitOr", "BitXor", "Shl", "Shr",
		"LAnd", "LOr"
	};
	std::string_view getTypeString(BinaryOp type)
	{
		return s_BinaryOp[(u8)type];
	}

	static const char* s_UnaryOp[] = {
		"Unknown",
		"Plus",           // kPlus
		"Minus",          // kMinus
		"LogicalNot",     // kBang
		"BitwiseNot",     // kTilde
		"PreIncrement",   // kPrePlus
		"PreDecrement",   // kPreMinus
		"PostIncrement",  // kPostPlus
		"PostDecrement",  // kPostMinus
		"AddressOf",      // kAmp
		"Dereference",    // kStar
		"Cast"            // kCast
	};
	std::string_view toTypeString(UnaryOp type)
	{
		return s_UnaryOp[(u8)type];
	}

	std::string NumberLiteralExpr::getNumberString() const
	{
		switch (m_type)
		{
		case LiteralType::kU8:
			return std::to_string(m_value.u8);
		case LiteralType::kU16:
			return std::to_string(m_value.u16);
		case LiteralType::kU32:
			return std::to_string(m_value.u32);
		case LiteralType::kU64:
			return std::to_string(m_value.u64);
		case LiteralType::kI8:
			return std::to_string(m_value.i8);
		case LiteralType::kI16:
			return std::to_string(m_value.i16);
		case LiteralType::kI32:
			return std::to_string(m_value.i32);
		case LiteralType::kI64:
			return std::to_string(m_value.i64);
		case LiteralType::kF32:
			return std::to_string(m_value.f32);
		case LiteralType::kF64:
			return std::to_string(m_value.f64);
		case LiteralType::kF128:
			return std::to_string(m_value.f128);
		case LiteralType::kUnknown:
		case LiteralType::kBool:
			return "";
		}
	}

	Expected<NumberLiteralExpr*> NumberLiteralExpr::parseFloatToken(const std::string& num)
	{
		std::string lower;
		lower.reserve(num.size());
		for (char c : num)
			lower += std::tolower(c);

		LiteralType ty = LiteralType::kF64; // f64 first
		std::string numStr = lower;

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
				float val = std::stof(numStr);
				return new NumberLiteralExpr(val);
			}
			case LiteralType::kF64:
			{
				double val = std::stod(numStr);
				return new NumberLiteralExpr(val);
			}
			case LiteralType::kF128:
			{
				long double val = std::stold(numStr);
				return new NumberLiteralExpr(val);
			}
			default:
				return Result::failure("Unknown float literal type");
			}
		}
		catch (const std::exception& e)
		{
			return Result::failure(std::string("Invalid float literal: ") + e.what());
		}
	}

	Expected<NumberLiteralExpr*> NumberLiteralExpr::parseNumberToken(const std::string& txt)
	{
		std::string num = txt;
		bool isUnsigned = false;
		bool isLong = false;
		bool isLongLong = false;

		// check suffix
		std::string lower;
		for (char c : num) lower += std::tolower(c);
		if (lower.ends_with("ull") || lower.ends_with("llu")) { isUnsigned = true; isLongLong = true; }
		else if (lower.ends_with("ul") || lower.ends_with("lu")) { isUnsigned = true; isLong = true; }
		else if (lower.ends_with("u")) { isUnsigned = true; }
		else if (lower.ends_with("l")) { isLong = true; }

		// remove suffix
		auto pos = num.find_first_not_of("0123456789xXabcdefABCDEF");
		if (pos != std::string::npos) num = num.substr(0, pos);

		// check
		int base = 10;
		if (num.rfind("0x", 0) == 0 || num.rfind("0X", 0) == 0) base = 16;
		else if (num.rfind("0", 0) == 0 && num.size() > 1) base = 8;

		unsigned long long val = 0;
		try {
			val = std::stoull(num, nullptr, base);
		} catch (...) {
			return Result::failure("Invalid integer literal: " + num);
		}

		// check number type
		LiteralType ty = LiteralType::kUnknown;

		if (!isUnsigned && base == 10)
		{
			if (val <= kMaxI32) ty = LiteralType::kI32;
			else if (val <= kMaxI64) ty = LiteralType::kI64;
		}
		else // hex or oct or unsigned suffix
		{
			if (val <= kMaxU32) ty = LiteralType::kU32;
			else if (val <= kMaxU64) ty = LiteralType::kU64;
		}

		if (isLong && ty == LiteralType::kI32) ty = LiteralType::kI64;
		if (isLongLong) ty = LiteralType::kI64;
		if (isUnsigned && ty == LiteralType::kI32) ty = LiteralType::kU32;

		// fallback
		if (ty == LiteralType::kUnknown) ty = LiteralType::kI64;

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

	BoolLiteralExpr::BoolLiteralExpr(bool val)
		: ASTExpr(ExprKind::kBoolLit)
		, value{ val }
	{
	}

	BinaryExpr::BinaryExpr(BinaryOp type, ASTExpr* l, ASTExpr* r)
		: ASTExpr(ExprKind::kBinary)
		, op{ type }
		, left{ l }
		, right{ r }
	{
	}

	UnaryExpr::UnaryExpr(UnaryOp op, ASTExpr* expr)
		: ASTExpr(ExprKind::kUnary)
		, op{ op }
		, operand{ expr }
	{
	}

	CallExpr::CallExpr(ASTExpr* fTag, std::vector<ASTExpr*> args)
		: ASTExpr(ExprKind::kFuncCall)
		, funcTag{ fTag }
		, callArgs{ std::move(args) }
	{
	}

	CallExpr::CallExpr(ASTExpr* fTag, std::initializer_list<ASTExpr*> args)
		: ASTExpr(ExprKind::kFuncCall)
		, funcTag{ fTag }
		, callArgs{ args }
	{
	}

	CallExpr::CallExpr()
		: ASTExpr(ExprKind::kFuncCall)
		, funcTag{ nullptr }
		, callArgs{}
	{
	}

	MemberAccessExpr::MemberAccessExpr(ASTExpr* object, std::string member)
		: ASTExpr(ExprKind::kMemberAccess)
		, objectExpr{ object }
		, member{ std::move(member) }
	{
	}

	MemberAccessExpr::MemberAccessExpr()
		: ASTExpr(ExprKind::kMemberAccess)
		, objectExpr{ nullptr }
		, member{}
	{
	}

	VariableRefExpr::VariableRefExpr(std::string name)
		: ASTExpr(ExprKind::kVar)
		, variableName{ std::move(name) }
	{
	}

	VariableRefExpr::VariableRefExpr()
		: ASTExpr(ExprKind::kVar)
		, variableName{}
	{
	}

	CastExpr::CastExpr(ASTExpr* obj, ASTTypeNode* type)
		: ASTExpr(ExprKind::kCast)
		, castTo{ type }
		, objectExpr{ obj }
	{
	}

	CastExpr::CastExpr()
		: ASTExpr(ExprKind::kCast)
		, castTo{ nullptr }
		, objectExpr{ nullptr }
	{

	}

	NewExpr::NewExpr()
		: ASTExpr(ExprKind::kNew)
	{
	}

	NewExpr::NewExpr(ASTTypeNode* type, std::vector<ASTExpr*> args)
		: ASTExpr(ExprKind::kNew)
		, type{ type }
		, arguments{ std::move(args) }
	{
	}

	NewExpr::NewExpr(ASTTypeNode* type, std::initializer_list<ASTExpr*> args)
		: ASTExpr(ExprKind::kNew)
		, type{ type }
		, arguments{ args }
	{
	}

	StringLiteralExpr::StringLiteralExpr()
		: ASTExpr(ExprKind::kStringLit)
		, value{}
	{
	}

	StringLiteralExpr::StringLiteralExpr(std::string s)
		: ASTExpr(ExprKind::kStringLit)
		, value{ std::move(s) }
	{
	}

	StringLiteralExpr::StringLiteralExpr(const std::string_view s)
		: ASTExpr(ExprKind::kStringLit)
		, value{ s }
	{
	}


	CharLiteralExpr::CharLiteralExpr()
	    : ASTExpr(ExprKind::kCharLit)
		, value {}
	{
	}

	CharLiteralExpr::CharLiteralExpr(char c)
	    : ASTExpr(ExprKind::kCharLit)
		, value {c}
	{
	}

	LambdaFuncExpr::LambdaFuncExpr()
	    : ASTExpr(ExprKind::kLambda)
		, function {}
	{

	}

	LambdaFuncExpr::LambdaFuncExpr(struct FuncDecl* func)
		: ASTExpr(ExprKind::kLambda)
		, function {func}
	{

	}

	ArrayLiteralExpr::ArrayLiteralExpr()
	    : ASTExpr(ExprKind::kArrayLit)
		, elements {}
	{

	}

	ArrayLiteralExpr::ArrayLiteralExpr(std::vector<ASTExpr*> v)
		: ASTExpr(ExprKind::kArrayLit)
		, elements {std::move(v)}
	{

	}

	ArrayLiteralExpr::~ArrayLiteralExpr()
	{
		//TODO clean up
	}
}