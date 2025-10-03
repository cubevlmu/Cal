// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. 
// 
// TypeTraits.hpp - Lightweight type traits & utilities

#pragma once

#include <nbase/common.hpp>

namespace neo {

    ///////////////////////////////////////////////////////////////
    /// Basic bool constants
	///////////////////////////////////////////////////////////////

	template<bool B> struct NBoolConstant { static constexpr bool Value = B; };
	using NTrue  = NBoolConstant<true>;
	using NFalse = NBoolConstant<false>;

	///////////////////////////////////////////////////////////////
    /// Compare operators
	///////////////////////////////////////////////////////////////

	template<typename A, typename B> struct NIsSame : NFalse {};
	template<typename T> struct NIsSame<T, T> : NTrue {};

	///////////////////////////////////////////////////////////////
    /// Logic operators
	///////////////////////////////////////////////////////////////

	template<typename...> struct NAnd : NTrue {};
	template<typename A, typename... Rest>
	struct NAnd<A, Rest...> : NBoolConstant<A::Value && NAnd<Rest...>::Value> {};

	template<typename...> struct NOr : NFalse {};
	template<typename A, typename... Rest>
	struct NOr<A, Rest...> : NBoolConstant<A::Value || NOr<Rest...>::Value> {};

	template<typename T> struct NNot : NBoolConstant<!T::Value> {};

	template<bool Left, typename... Right>
	struct NOrValue { enum { Value = NOr<Right...>::Value }; };
	template<typename... Right>
	struct NOrValue<true, Right...> { enum { Value = true }; };

	template<typename X, typename Y>
	struct NAreTypesEqual { enum { Value = false }; };
	template<typename T>
	struct NAreTypesEqual<T, T> { enum { Value = true }; };

	///////////////////////////////////////////////////////////////
    /// Type-traits
	///////////////////////////////////////////////////////////////

	template<typename T> struct NIsEnum : NBoolConstant<__is_enum(T)> {};
	template<typename T> struct NIsPointer : NFalse {};
	template<typename T> struct NIsPointer<T*> : NTrue {};

	template<typename T> struct NIsLValueRef : NFalse {};
	template<typename T> struct NIsLValueRef<T&> : NTrue {};

	template<typename T> struct NIsRValueRef : NFalse {};
	template<typename T> struct NIsRValueRef<T&&> : NTrue {};

	template<typename T>
	struct NIsReference : NBoolConstant<NIsLValueRef<T>::Value || NIsRValueRef<T>::Value> {};

	template<typename T> struct NIsVoid : NFalse {};
	template<> struct NIsVoid<void> : NTrue {};
	template<> struct NIsVoid<const void> : NTrue {};
	template<> struct NIsVoid<volatile void> : NTrue {};
	template<> struct NIsVoid<const volatile void> : NTrue {};

	///////////////////////////////////////////////////////////////
    /// Removing or adding modifiers
	///////////////////////////////////////////////////////////////

	template<typename T> struct NRemoveCV { using Type = T; };
	template<typename T> struct NRemoveCV<const T> { using Type = T; };
	template<typename T> struct NRemoveCV<volatile T> { using Type = T; };
	template<typename T> struct NRemoveCV<const volatile T> { using Type = T; };

	template<typename T> struct NRemoveRef { using Type = T; };
	template<typename T> struct NRemoveRef<T&> { using Type = T; };
	template<typename T> struct NRemoveRef<T&&> { using Type = T; };

	template<typename T> struct NAddConst { using Type = const T; };
	template<typename T> struct NAddCV { using Type = const volatile T; };

	///////////////////////////////////////////////////////////////
    /// Constructs & assignment
	///////////////////////////////////////////////////////////////

	template<typename T>
	struct NIsCopyConstructible
		: NBoolConstant<__is_constructible(T, const T&)> {};

	template<typename T>
	struct NIsTriviallyCopyConstructible
		: NBoolConstant<__is_trivially_copyable(T)> {};

	template<typename T>
	struct NIsTriviallyCopyAssignable
		: NBoolConstant<__is_trivially_assignable(T&, const T&)> {};

	template<class Base, class Derived>
	struct NIsBaseOf : NBoolConstant<__is_base_of(Base, Derived)> {};

    template<typename T>
    struct NIsPODType { enum { Value = NOrValue<__is_pod(T) || __is_enum(T), NIsPointer<T>>::Value }; };

	template<typename T>
	struct NIsTriviallyConstructible { enum { Value = NIsPODType<T>::Value }; };
	template <typename T, bool IsTriviallyDestructible = __is_enum(T)>
	struct NIsTriviallyDestructible { enum { Value = true }; };
	template <typename T>
	struct NIsTriviallyDestructible<T, false> { enum { Value = __is_trivially_destructible(T) }; };

	template<typename T, typename Arg>
	struct NIsBitwiseConstructible {
		static_assert(!NIsReference<T>::Value && !NIsReference<Arg>::Value, "NIsBitwiseConstructible cannot use reference types");
		static_assert(NAreTypesEqual<T, typename NRemoveCV<T>::Type>::Value&& NAreTypesEqual<Arg, typename NRemoveCV<Arg>::Type>::Value, "NIsBitwiseConstructible cannot use qualified types");
		enum { Value = false };
	};
	template<typename T>
	struct NIsBitwiseConstructible<T, T> {
		enum { Value = NIsTriviallyCopyConstructible<T>::Value };
	};
	template<typename T, typename U>
	struct NIsBitwiseConstructible<const T, U> : NIsBitwiseConstructible<T, U> {

	};
	template<typename T> struct NIsBitwiseConstructible<const T*, T*> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<u8, i8> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<i8, u8> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<u16, i16> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<i16, u16> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<u32, i32> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<i32, u32> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<u64, i64> {
		enum { Value = true };
	};
	template<> struct NIsBitwiseConstructible<i64, u64> {
		enum { Value = true };
	};

	///////////////////////////////////////////////////////////////
    /// Conditional function
	///////////////////////////////////////////////////////////////

	template<bool Condition, typename T = void> struct NEnableIf {};
	template<typename T> struct NEnableIf<true, T> { using Type = T; };

	///////////////////////////////////////////////////////////////
	/// Utils
	///////////////////////////////////////////////////////////////

	template<typename T>
	inline typename NRemoveRef<T>::Type&& NMove(T&& obj) noexcept {
		return static_cast<typename NRemoveRef<T>::Type&&>(obj);
	}

	template<typename T>
	inline T&& NForward(typename NRemoveRef<T>::Type& t) noexcept {
		return static_cast<T&&>(t);
	}
	template<typename T>
	inline T&& NForward(typename NRemoveRef<T>::Type&& t) noexcept {
		return static_cast<T&&>(t);
	}
	template<typename T>
	inline void NSwap(T& a, T& b) noexcept {
		T tmp = NMove(a);
		a = NMove(b);
		b = NMove(tmp);
	}

	///////////////////////////////////////////////////////////////
    /// Bit operate templates
	///////////////////////////////////////////////////////////////

	inline unsigned int NReverseBits(unsigned int bits) {
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
		bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
		return bits;
	}

	///////////////////////////////////////////////////////////////
    /// Elect float/double values
	///////////////////////////////////////////////////////////////

	template<typename T> struct NOtherFloat { using Type = float; };
	template<> struct NOtherFloat<float> { using Type = double; };

} // namespace neo
