/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <stdint.h>
#include <ranges>
#include <string>

#include <nbase/environment.hpp>

namespace neo {

	typedef int8_t      i8;
	typedef uint8_t     u8;
	typedef int16_t     i16;
	typedef uint16_t    u16;
	typedef int32_t     i32;
	typedef uint32_t    u32;
	typedef int64_t     i64;
	typedef uint64_t    u64;
	typedef float       f32;
	typedef double      f64;
	typedef long double f128;

	typedef size_t  psize;
	typedef u8      byte;
	typedef char    sbyte;
	typedef wchar_t wchar;

	constexpr i8 kMaxI8 = std::numeric_limits<i8>::max();
	constexpr i8 kMinI8 = std::numeric_limits<i8>::min();
	constexpr i16 kMaxI16 = std::numeric_limits<i16>::max();
	constexpr i16 kMinI16 = std::numeric_limits<i16>::min();
	constexpr i32 kMaxI32 = std::numeric_limits<i32>::max();
	constexpr i32 kMinI32 = std::numeric_limits<i32>::min();
	constexpr i64 kMaxI64 = std::numeric_limits<i64>::max();
	constexpr i64 kMinI64 = std::numeric_limits<i64>::min();

	constexpr u8 kMaxU8 = std::numeric_limits<u8>::max();
	constexpr u8 kMinU8 = std::numeric_limits<u8>::min();
	constexpr u16 kMaxU16 = std::numeric_limits<u16>::max();
	constexpr u16 kMinU16 = std::numeric_limits<u16>::min();
	constexpr u32 kMaxU32 = std::numeric_limits<u32>::max();
	constexpr u32 kMinU32 = std::numeric_limits<u32>::min();
	constexpr u64 kMaxU64 = std::numeric_limits<u64>::max();
	constexpr u64 kMinU64 = std::numeric_limits<u64>::min();

	constexpr psize kMaxPSize = std::numeric_limits<psize>::max();
	constexpr psize kMinPSize = std::numeric_limits<psize>::min();
	static_assert(sizeof(psize) == sizeof(void*), "psize is not vailed!");

	constexpr f32 kF32Max = std::numeric_limits<f32>::max();
	constexpr f32 kF32Min = -std::numeric_limits<f32>::max();
	constexpr f64 kF64Max = std::numeric_limits<f64>::max();
	constexpr f64 kF64Min = -std::numeric_limits<f64>::max();
	static_assert(sizeof(bool) == 1, "bool is not vailed!");

	/// Get the length of an stack based or heap based array
	template <typename T, u32 count>
	constexpr u32 lengthOf(const T(&)[count])
	{
		return count;
	}

	/// Version struct for presentating an version info
	typedef struct {
		i32 major;
		i32 minor;
		i32 reverse;
	} Version;

	/// Fetech the neo-base module's version
	void getNeoBaseVersion(Version&);

}