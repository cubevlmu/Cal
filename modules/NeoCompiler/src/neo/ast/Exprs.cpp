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
		"f32", "f64", "bool"
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
		"Plus", "Minus", "LogicalNot", "BitwiseNot",
		"PreIncrement", "PreDecrement", "PostIncrement", "PostDecrement"
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
		case LiteralType::kUnknown:
		case LiteralType::kBool:
			return "";
		}
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
}