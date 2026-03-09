/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/memory/Memory.hpp>

namespace neo {

	class NString;
	class NStringView;
	template <typename T>
	class NSpan;

	namespace strings {

		// --- Misc ---

		int length(const char* str);
		// skip '\0' until found real '\0'
		NString skipZeroToZero(const char* str);
		i32 hexDigit(char c);

		u32 getStringsHash(const char* str);
		u32 getStringsHash(const char* str, i32 length);

		// --- Character Utilities ---

		char toLower(char c);
		char toUpper(char c);

		bool isUpper(char c);
		bool isLower(char c);
		bool isAlpha(char c);
		bool isPunct(char c);
		bool isAlnum(char c);
		bool isDigit(char c);
		bool isWhitespace(char c);
		bool isHexDigit(char c);

		// --- Search Functions ---

		const char* find(const char* str, char toFind);
		const char* find(NStringView str, char toFind);
		const char* find(const char* str, const char* toFind);
		const char* find(NStringView str, NStringView toFind);

		const char* findInsensitive(const char* str, const char* toFind);
		const char* findInsensitive(NStringView str, NStringView toFind);

		const char* reverseFind(NStringView str, char c);
		bool contains(NStringView str, char toFind);

		// --- Equality ---
		bool equalStrings(const char* lhs, const char* rhs);
		bool equalStrings(const char* lhs, const char* rhs, size_t len);
		bool equalStrings(NStringView lhs, NStringView rhs);
		bool equalStrings(NStringView lhs, NStringView rhs, size_t len);

		bool equalIStrings(const char* lhs, const char* rhs);
		bool equalIStrings(const char* lhs, const char* rhs, size_t len);
		bool equalIStrings(NStringView lhs, NStringView rhs);
		bool equalIStrings(NStringView lhs, NStringView rhs, size_t len);

		// --- Comparison ---

		i32 compareStrings(const char* str1, const char* str2);
		i32 compareStrings(const char* str1, const char* str2, i32 maxCount);
		i32 compareIStrings(const char* str1, const char* str2);
		i32 compareIStrings(const char* str1, const char* str2, i32 maxCount);

		// --- Prefix/Suffix ---

		bool startsWith(NStringView str, NStringView prefix);
		bool startsWithInsensitive(NStringView str, NStringView prefix);
		bool endsWith(NStringView str, NStringView suffix);
		bool endsWithInsensitive(NStringView str, NStringView suffix);

		// --- Conversion To NString ---

		char* toString(bool value, NSpan<char> output);
		char* toString(i32 value, NSpan<char> output);
		char* toString(i64 value, NSpan<char> output);
		char* toString(u32 value, NSpan<char> output);
		char* toString(u64 value, NSpan<char> output);
		char* toString(f32 value, NSpan<char> output, int after_point);
		char* toString(f64 value, NSpan<char> output, int after_point);

		void toStringHex(u8 value, NSpan<char> output);
		void toStringPretty(i32 value, NSpan<char> output);
		void toStringPretty(u32 value, NSpan<char> output);
		void toStringPretty(u64 value, NSpan<char> output);

		NString toString(i32 value);
		NString toString(i64 value);
		NString toString(u32 value);
		NString toString(u64 value);
		// TODO need test
		NString toString(f32 value);
		NString toString(f64 value);

		// --- Conversion From NString ---

		bool fromString(NStringView str, u64& result);
		bool fromString(NStringView str, u32& result);
		bool fromString(NStringView str, u16& result);
		bool fromString(NStringView str, u8& result);
		bool fromString(NStringView str, i64& result);
		bool fromString(NStringView str, i32& result);
		bool fromString(NStringView str, i16& result);
		bool fromString(NStringView str, i8& result);
		bool fromString(NStringView str, f32& result);
		bool fromString(NStringView str, bool& value);

		bool fromHexString(const char* str, i32 length, u32& result);

		const char* fromStringOctal(NStringView str, u32& value);

		// --- Copy/Concat/Transform ---

		char* copyString(NSpan<char> output, NStringView source);
		char* catString(NSpan<char> output, NStringView source);
		bool makeLowercase(NSpan<char> output, NStringView source);

		// --- Other utility functions ---

		NE_FORCE_INLINE bool fromHexString(const char* str, u32& result)
		{
			return strings::fromHexString(str, length(str), result);
		}
		// supporting cobalt hashing model
		NE_FORCE_INLINE u32 getHash(const char* key) {
			return strings::getStringsHash(key);
		}
		NE_FORCE_INLINE u32 getHash(const char* key, i32 length) {
			return strings::getStringsHash(key, length);
		}
	}

#define CONCAT_STR(NAME, ...) constexpr const char* NAME = ::neo::concatStr(__VA_ARGS__)

    void replaceAll(String& str, const char* from, const char* to);
    void splitStr(Vector<String>& out, const String& str, const char* delim, bool skipEmpty = true);
    void splitStr(Vector<String>& out, const String& str, char delim, bool skipEmpty = true);
    void splitAt(Vector<String>& out, int pos, const String& str);
    int findLast(const String& str, char hint);
    
    template <typename... T>
    String msg(T&&... args) {
	    OStringStream oss {};
        (oss << ... << args);
        return oss.str();
    }

    template <typename... T>
    String concatStr(const T&... args) {
        String r;
        auto append = [](String* s, const char* v) {
            s->append(v);
        };
        (append(&r, args), ...);
        return r;
    }
}