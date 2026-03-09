/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>

namespace neo {

    ///////////////////////////////////////////////////////////////
    /// Basic bool constants
	///////////////////////////////////////////////////////////////

	namespace impl
	{
		template <typename T, bool IsTriviallyDestructible = __is_enum(T)>
		struct NIsTriviallyDestructibleImpl
		{
			enum { Value = true };
		};

		template <typename T>
		struct NIsTriviallyDestructibleImpl<T, false>
		{
#if defined(__clang__) && __clang_major__ >= 15
			enum { Value = __is_trivially_destructible(T) };
#else
			enum { Value = __has_trivial_destructor(T) };
#endif
		};
	}

	////////////////////////////////////////////////////////////////////////////////////

	// Performs boolean AND operation.

	template<typename... Types>
	struct NAnd;

	template<bool Left, typename... Right>
	struct NAndValue
	{
		enum { Value = NAnd<Right...>::Value };
	};

	template<typename... Right>
	struct NAndValue<false, Right...>
	{
		enum { Value = false };
	};

	template<typename Left, typename... Right>
	struct NAnd<Left, Right...> : NAndValue<Left::Value, Right...>
	{
	};

	template<>
	struct NAnd<>
	{
		enum { Value = true };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Performs boolean OR operation.

	template<typename... Types>
	struct NOr;

	template<bool Left, typename... Right>
	struct NOrValue
	{
		enum { Value = NOr<Right...>::Value };
	};

	template<typename... Right>
	struct NOrValue<true, Right...>
	{
		enum { Value = true };
	};

	template<typename Left, typename... Right>
	struct NOr<Left, Right...> : NOrValue<Left::Value, Right...>
	{
	};

	template<>
	struct NOr<>
	{
		enum { Value = false };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Performs boolean NOT operation.

	template<typename Type>
	struct NNot
	{
		enum { Value = !Type::Value };
	};

	////////////////////////////////////////////////////////////////////////////////////

	template<typename A, typename B> struct NIsTheSame { enum { Value = false }; };
	template<typename T>             struct NIsTheSame<T, T> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T> struct NIsLValueReference { enum { Value = false }; };
	template<typename T> struct NIsLValueReference<T&> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T> struct NIsRValueReferenceType { enum { Value = false }; };
	template<typename T> struct NIsRValueReferenceType<T&&> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T> struct NIsReferenceType { enum { Value = false }; };
	template<typename T> struct NIsReferenceType<T&> { enum { Value = true }; };
	template<typename T> struct NIsReferenceType<T&&> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T> struct NIsVoidType { enum { Value = false }; };
	template<> struct NIsVoidType<void> { enum { Value = true }; };
	template<> struct NIsVoidType<void const> { enum { Value = true }; };
	template<> struct NIsVoidType<void volatile> { enum { Value = true }; };
	template<> struct NIsVoidType<void const volatile> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type is a pointer.

	template<typename T> struct NIsPointer { enum { Value = false }; };
	template<typename T> struct NIsPointer<T*> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type is an enum.

	template<typename T>
	struct NIsEnum
	{
		enum { Value = __is_enum(T) };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type is POD (plain old data type).

	template<typename T>
	struct NIsPODType
	{
		enum { Value = NOrValue<__is_pod(T) || __is_enum(T), NIsPointer<T>>::Value };
	};

	////////////////////////////////////////////////////////////////////////////////////

	template<class Base, class Derived>
	struct NIsBaseOf
	{
		enum { Value = __is_base_of(Base, Derived) };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Removes any const or volatile qualifiers from a type.

	template<typename T> struct NRemoveCV { typedef T Type; };
	template<typename T> struct NRemoveCV<const T> { typedef T Type; };
	template<typename T> struct NRemoveCV<volatile T> { typedef T Type; };
	template<typename T> struct NRemoveCV<const volatile T> { typedef T Type; };

	////////////////////////////////////////////////////////////////////////////////////

	// Removes any reference qualifiers from a type.

	template<typename T> struct NRemoveReference { typedef T Type; };
	template<typename T> struct NRemoveReference<T&> { typedef T Type; };
	template<typename T> struct NRemoveReference<T&&> { typedef T Type; };

	////////////////////////////////////////////////////////////////////////////////////

	// Removes any const qualifiers from a type.

	template<typename T> struct NRemoveConst { typedef T Type; };
	template<typename T> struct NRemoveConst<const T> { typedef T Type; };

	////////////////////////////////////////////////////////////////////////////////////

	// Adds qualifiers to a type.

	template<typename T> struct NAddCV { typedef const volatile T Type; };
	template<typename T> struct NAddConst { typedef const T Type; };

	////////////////////////////////////////////////////////////////////////////////////

	// Creates a lvalue or rvalue reference type.

	namespace THelpers
	{
		template<typename T>
		struct NTtypeIdentity { using Type = T; };

		template<typename T>
		auto TTryAddLValueReference(int) -> NTtypeIdentity<T&>;
		template <typename T>
		auto TTryAddLValueReference(...) -> NTtypeIdentity<T>;

		template<typename T>
		auto TTryAddRValueReference(int) -> NTtypeIdentity<T&&>;
		template<typename T>
		auto TTryAddRValueReference(...) -> NTtypeIdentity<T>;
	}

	template<typename T>
	struct NAddLValueReference : decltype(THelpers::TTryAddLValueReference<T>(0))
	{
	};

	template<typename T>
	struct NAddRValueReference : decltype(THelpers::TTryAddRValueReference<T>(0))
	{
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type has a copy constructor.

	template<typename T>
	struct NIsCopyConstructible
	{
		enum { Value = __is_constructible(T, typename NAddLValueReference<typename NAddConst<T>::Type>::Type) };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type has a trivial copy constructor.

	template<typename T>
	struct NIsTriviallyCopyConstructible
	{
#if defined(__clang__) && __clang_major__ >= 15
		enum { Value = NOrValue<__is_trivially_copyable(T), NIsPODType<T>>::Value };
#else
		enum { Value = NOrValue<__has_trivial_copy(T), NIsPODType<T>>::Value };
#endif
	};

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	struct NIsTriviallyConstructible
	{
		enum { Value = NIsPODType<T>::Value };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Check if a type has a trivial destructor.

	template <typename T>
	struct NIsTriviallyDestructible
	{
		enum { Value = impl::NIsTriviallyDestructibleImpl<T>::Value };
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type has a trivial copy assignment operator.

	template<typename T>
	struct NIsTriviallyCopyAssignable
	{
#if defined(__clang__) && __clang_major__ >= 15
		enum { Value = NOrValue<__is_trivially_assignable(T, const T), NIsPODType<T>>::Value };
#else
		enum { Value = NOrValue<__has_trivial_assign(T), NIsPODType<T>>::Value };
#endif
	};

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T>                           struct NIsFunction { enum { Value = false }; };
	template<typename RetType, typename... Params> struct NIsFunction<RetType(Params...)> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	template<typename X, typename Y> struct NAreTypesEqual { enum { Value = false }; };
	template<typename T> struct NAreTypesEqual<T, T> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	inline typename NRemoveReference<T>::Type&& moveTemp(T&& obj)
	{
		return (typename NRemoveReference<T>::Type&&)obj;
	}

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	inline void swapVal(T& a, T& b) noexcept
	{
		T tmp = moveTemp(a);
		a = moveTemp(b);
		b = moveTemp(tmp);
	}

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	inline T&& forward(typename NRemoveReference<T>::Type& t) noexcept
	{
		return static_cast<T&&>(t);
	}

	template<typename T>
	inline T&& forward(typename NRemoveReference<T>::Type&& t) noexcept
	{
		return static_cast<T&&>(t);
	}

	////////////////////////////////////////////////////////////////////////////////////

	template<bool Condition, typename TrueResult, typename FalseResult>
	struct NStaticIf;

	template<typename TrueResult, typename FalseResult>
	struct NStaticIf<true, TrueResult, FalseResult>
	{
		typedef TrueResult Value;
	};

	template<typename TrueResult, typename FalseResult>
	struct NStaticIf<false, TrueResult, FalseResult>
	{
		typedef FalseResult Value;
	};

	////////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	struct NRemovePointer
	{
		typedef T Type;
	};

	template<typename T>
	struct NRemovePointer<T*>
	{
		typedef typename NRemovePointer<T>::Type Type;
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Includes a function in an overload set if the predicate is true.

	template<bool Predicate, typename Result = void>
	struct NEnableIf;

	template<typename Result>
	struct NEnableIf<true, Result>
	{
		typedef Result Type;
	};

	template<typename Result>
	struct NEnableIf<false, Result>
	{
	};

	////////////////////////////////////////////////////////////////////////////////////

	// Reverses the order of the bits of a value.
	template<typename T>
	inline typename NEnableIf<NAreTypesEqual<T, unsigned int>::Value, T>::Type reverseBits(T bits)
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
		bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
		return bits;
	}

	////////////////////////////////////////////////////////////////////////////////////

	// Checks if a type T is bitwise-constructible from a given argument type U. Can be used to perform a fast memory copy instead of slower constructor invocations.

	template<typename T, typename Arg>
	struct NIsBitwiseConstructible
	{
		static_assert(!NIsReferenceType<T>::Value && !NIsReferenceType<Arg>::Value, "NIsBitwiseConstructible cannot use reference types");
		static_assert(NAreTypesEqual<T, typename NRemoveCV<T>::Type>::Value&& NAreTypesEqual<Arg, typename NRemoveCV<Arg>::Type>::Value, "NIsBitwiseConstructible cannot use qualified types");
		enum { Value = false };
	};

	template<typename T>
	struct NIsBitwiseConstructible<T, T>
	{
		enum { Value = NIsTriviallyCopyConstructible<T>::Value };
	};

	template<typename T, typename U>
	struct NIsBitwiseConstructible<const T, U> : NIsBitwiseConstructible<T, U>
	{
	};

	template<typename T>
	struct NIsBitwiseConstructible<const T*, T*>
	{
		enum { Value = true };
	};

	template<> struct NIsBitwiseConstructible<u8, i8> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<i8, u8> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<u16, i16> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<i16, u16> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<u32, i32> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<i32, u32> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<u64, i64> { enum { Value = true }; };
	template<> struct NIsBitwiseConstructible<i64, u64> { enum { Value = true }; };

	////////////////////////////////////////////////////////////////////////////////////

	// Utility to select double for float type or float otherwise

	template<typename T>
	struct NOtherFloat { typedef float Type; };
	template<>
	struct NOtherFloat<float> { typedef double Type; };

} // namespace neo
