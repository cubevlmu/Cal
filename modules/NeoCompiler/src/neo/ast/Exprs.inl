/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <neo/ast/Exprs.hpp>

namespace neo
{

	template <typename T>
	NumberLiteralExpr::NumberLiteralExpr(T num)
		: ASTExpr(ExprKind::kNumberLit)
	{
		setNumber(num);
	}

	template <typename T>
	T NumberLiteralExpr::getNumber()
	{
		switch (m_type)
		{
		case LiteralType::kU8:
			if constexpr (std::is_same_v<T, u8>)
				return m_value.u8;
			break;
		case LiteralType::kU16:
			if constexpr (std::is_same_v<T, u16>)
				return m_value.u16;
			break;
		case LiteralType::kU32:
			if constexpr (std::is_same_v<T, u32>)
				return m_value.u32;
			break;
		case LiteralType::kU64:
			if constexpr (std::is_same_v<T, u64>)
				return m_value.u64;
			break;
		case LiteralType::kI8:
			if constexpr (std::is_same_v<T, i8>)
				return m_value.i8;
			break;
		case LiteralType::kI16:
			if constexpr (std::is_same_v<T, i16>)
				return m_value.i16;
			break;
		case LiteralType::kI32:
			if constexpr (std::is_same_v<T, i32>)
				return m_value.i32;
			break;
		case LiteralType::kI64:
			if constexpr (std::is_same_v<T, i64>)
				return m_value.i64;
			break;
		case LiteralType::kF32:
			if constexpr (std::is_same_v<T, f32>)
				return m_value.f32;
			break;
		case LiteralType::kF64:
			if constexpr (std::is_same_v<T, f64>)
				return m_value.f64;
			break;
		case LiteralType::kF128:
			if constexpr (std::is_same_v<T, f128>)
				return m_value.f128;
		default:
			NE_ASSERT(false && "type no support");
		}
	}

	template <typename T>
	void NumberLiteralExpr::setNumber(T num)
	{
		if constexpr (std::is_same_v<T, i8>)
		{
			m_value.i8 = num;
			m_type = LiteralType::kI8;
		}
		else if constexpr (std::is_same_v<T, i16>)
		{
			m_value.i16 = num;
			m_type = LiteralType::kI16;
		}
		else if constexpr (std::is_same_v<T, i32>)
		{
			m_value.i32 = num;
			m_type = LiteralType::kI32;
		}
		else if constexpr (std::is_same_v<T, i64>)
		{
			m_value.i64 = num;
			m_type = LiteralType::kI64;
		}
		else if constexpr (std::is_same_v<T, u8>)
		{
			m_value.u8 = num;
			m_type = LiteralType::kU8;
		}
		else if constexpr (std::is_same_v<T, u16>)
		{
			m_value.u16 = num;
			m_type = LiteralType::kU16;
		}
		else if constexpr (std::is_same_v<T, u32>)
		{
			m_value.u32 = num;
			m_type = LiteralType::kU32;
		}
		else if constexpr (std::is_same_v<T, u64>)
		{
			m_value.u64 = num;
			m_type = LiteralType::kU64;
		}
		else if constexpr (std::is_same_v<T, f32>)
		{
			m_value.f32 = num;
			m_type = LiteralType::kF32;
		}
		else if constexpr (std::is_same_v<T, f64>)
		{
			m_value.f64 = num;
			m_type = LiteralType::kF64;
		}
		else if constexpr (std::is_same_v<T, f128>)
		{
			m_value.f128 = num;
			m_type = LiteralType::kF128;
		}
		else
		{
			NE_ASSERT(false && "not support");
		}
	}

}