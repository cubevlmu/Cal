// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "StringUtils.hpp"

#include "nbase/types/StringView.hpp"
#include "nbase/types/Span.hpp"
#include "nbase/types/String.hpp"

#if NE_WINDOWS
#else
#include <strings.h>
#endif

namespace neo {

	namespace strings {

		static char makeLowercase(char c) {
			return c >= 'A' && c <= 'Z' ? c - ('A' - 'a') : c;
		}


		i32 hexDigit(char c)
		{
			i32 result = 0;
			if (c >= '0' && c <= '9')
				result = c - '0';
			else if (c >= 'a' && c <= 'f')
				result = c + 10 - 'a';
			else if (c >= 'A' && c <= 'F')
				result = c + 10 - 'A';
			return result;
		}


		bool equalStrings(NStringView lhs, NStringView rhs)
		{
			if (rhs.length() != lhs.length()) return false;
			return strncmp(lhs.begin(), rhs.begin(), lhs.length()) == 0;
		}
		bool equalStrings(NStringView lhs, NStringView rhs, size_t len) {
			return equalStrings(lhs.begin(), rhs.begin(), len);
		}
		bool equalStrings(const char* lhs, const char* rhs) {
			return strcmp(lhs, rhs) == 0;
		}
		bool equalStrings(const char* lhs, const char* rhs, size_t len) {
			return strncmp(lhs, rhs, len) == 0;
		}


		bool equalIStrings(NStringView lhs, NStringView rhs)
		{
			if (lhs.length() != rhs.length()) return false;
			return equalIStrings(lhs.begin(), rhs.begin());
		}
		bool equalIStrings(const char* lhs, const char* rhs) {
#if NE_WINDOWS
			return _stricmp(lhs, rhs) == 0;
#else
			return strcasecmp(lhs, rhs) == 0;
#endif
		}
		bool equalIStrings(const char* lhs, const char* rhs, size_t len) {
#if NE_WINDOWS
			return _strnicmp(lhs, rhs, len) == 0;
#else
			return strncasecmp(lhs, rhs) == 0;
#endif
		}
		bool equalIStrings(NStringView lhs, NStringView rhs, size_t len) {
			return equalIStrings(lhs.begin(), rhs.begin(), len);
		}


		i32 compareStrings(const char* str1, const char* str2) {
			return strcmp(str1, str2);
		}
		i32 compareStrings(const char* str1, const char* str2, i32 maxCount) {
			return strncmp(str1, str2, maxCount);
		}
		i32 compareIStrings(const char* str1, const char* str2) {
#if NE_WINDOWS
			return _stricmp(str1, str2) == 0;
#else
			return strcasecmp(lhs, rhs) == 0;
#endif
		}
		i32 compareIStrings(const char* str1, const char* str2, i32 maxCount) {
#if NE_WINDOWS
			return _strnicmp(str1, str2, maxCount) == 0;
#else
			return strncasecmp(lhs, rhs) == 0;
#endif
		}


		int length(const char* str)
		{
			return (int)strlen(str);
		}


		bool endsWithInsensitive(NStringView str, NStringView suffix)
		{
			if (str.length() < suffix.length()) return false;
			NStringView fs{ str.end() - suffix.length(), suffix.length() };
			return equalIStrings(fs, suffix);
		}


		bool endsWith(NStringView str, NStringView suffix)
		{
			if (str.length() < suffix.length()) return false;
			NStringView fs{ str.end() - suffix.length(), suffix.length() };
			return equalStrings(fs, suffix);
		}


		const char* find(const char* str, char toFind)
		{
			const char* c = str;
			const char* end = str + sizeof(char) * length(str);
			while (c != end)
			{
				if (*c == toFind) return c;
				++c;
			}
			return nullptr;
		}


		const char* find(NStringView str, char toFind)
		{
			const char* c = str.begin();
			while (c != str.end())
			{
				if (*c == toFind) return c;
				++c;
			}
			return nullptr;
		}


		bool makeLowercase(NSpan<char> output, NStringView src)
		{
			char* destination = output.begin();
			if (src.length() + 1 > output.length()) return false;

			const char* source = src.begin();
			while (source != src.end()) {
				*destination = makeLowercase(*source);
				++destination;
				++source;
			}
			*destination = 0;
			return true;
		}


		const char* find(const char* str, const char* toFind) {
			return strstr(str, toFind);
		}
		const char* find(NStringView str, NStringView toFind) {
			return find(str.begin(), toFind.begin());
		}
		const char* findInsensitive(const char* str, const char* toFind)
		{
			if (toFind == nullptr || str == nullptr) {
				return nullptr;
			}

			const char findInitial = (char)toUpper(*toFind);
			const i32 length = strings::length(toFind++) - 1;
			char c = *str++;
			while (c) {
				c = (char)toUpper(c);
				if (c == findInitial && !compareIStrings(str, toFind, length)) {
					return str - 1;
				}

				c = *str++;
			}

			return nullptr;
		}
		const char* findInsensitive(NStringView str, NStringView toFind) {
			return findInsensitive(str.begin(), toFind.begin());
		}


		bool contains(NStringView haystack, char needle)
		{
			return find(haystack, needle) != nullptr;
		}


		char* copyString(NSpan<char> dst, NStringView src)
		{
			if (dst.length() < 1) return dst.begin();

			NE_ASSERT(dst.begin() >= src.end() || dst.begin() <= src.begin());

			u32 length = dst.length();
			char* tmp = dst.begin();
			const char* srcp = src.begin();
			while (srcp != src.end() && length > 1) {
				*tmp = *srcp;
				--length;
				++tmp;
				++srcp;
			}
			*tmp = 0;
			return tmp;
		}


		const char* reverseFind(NStringView haystack, char c)
		{
			if (haystack.length() == 0) return nullptr;

			const char* tmp = haystack.end() - 1;
			while (tmp >= haystack.begin()) {
				if (*tmp == c) return tmp;
				--tmp;
			}

			return nullptr;
		}


		char* catString(NSpan<char> destination, NStringView source)
		{
			char* dst = destination.begin();
			u32 length = destination.length();
			while (*dst && length) {
				--length;
				++dst;
			}
			return copyString(NSpan{ dst, static_cast<i32>(length) }, source);
		}


		static void reverse(char* str, int length)
		{
			char* beg = str;
			char* end = str + length - 1;
			while (beg < end)
			{
				char tmp = *beg;
				*beg = *end;
				*end = tmp;
				++beg;
				--end;
			}
		}


		bool fromString(NStringView str, u64& result)
		{
			i64 sum = 0;
			const char* p = str.begin();
			auto length = str.length();
			while (length--)
			{
				i32 c = *p++ - 48;
				if (c < 0 || c > 9)
					return false;
				sum = 10 * sum + c;
			}
			result = sum;
			return true;
		}


		bool fromString(NStringView str, u32& result)
		{
			u64 tmp;
			const bool b = fromString(str, tmp);
			result = (u32)tmp;
			return b;
		}


		bool fromString(NStringView str, u16& result)
		{
			u64 tmp;
			const bool b = fromString(str, tmp);
			result = (u16)tmp;
			return b;
		}


		bool fromString(NStringView str, u8& result)
		{
			u64 tmp;
			const bool b = fromString(str, tmp);
			result = (u8)tmp;
			return b;
		}


		bool fromString(NStringView str, i64& result)
		{
			i64 sum = 0;
			const char* p = str.begin();
			auto length = str.length();
			bool negate = false;
			while (length--)
			{
				i32 c = *p++ - 48;
				if (c == -3)
				{
					negate = true;
					continue;
				}
				if (c < 0 || c > 9)
					return false;
				sum = 10 * sum + c;
			}
			if (negate)
				sum = -sum;
			result = sum;
			return true;
		}


		bool fromString(NStringView str, i32& result)
		{
			i64 tmp;
			const bool b = fromString(str, tmp);
			result = (i32)tmp;
			return b;
		}


		bool fromString(NStringView str, i16& result)
		{
			i64 tmp;
			const bool b = fromString(str, tmp);
			result = (i16)tmp;
			return b;
		}


		bool fromString(NStringView str, i8& result)
		{
			i64 tmp;
			const bool b = fromString(str, tmp);
			result = (i8)tmp;
			return b;
		}


		bool fromString(NStringView str, f32& result)
		{
			char* end = nullptr;
			result = strtof(str.begin(), &end);
			return end != str.begin();
		}


		bool fromString(NStringView input, bool& value)
		{
			bool val = equalStrings(input, "true");
			value = val;
			return true;
		}


		const char* fromStringOctal(NStringView input, u32& value)
		{
			if (input.isEmpty()) return nullptr;

			const char* c = input.begin();
			value = 0;
			if (*c < '0' || *c > '7') return nullptr;

			while (c != input.end() && *c >= '0' && *c <= '7') {
				value *= 8;
				value += *c - '0';
				++c;
			}
			return c;
		}


		void toStringPretty(i32 value, NSpan<char> output)
		{
			char* c = output.begin();
			u32 length = output.length();
			if (length > 0)
			{
				if (value < 0)
				{
					value = -value;
					--length;
					*c = '-';
					++c;
				}
				return toStringPretty((unsigned int)value, NSpan(c, length));
			}
		}


		void toStringPretty(u32 value, NSpan<char> output)
		{
			return toStringPretty(u64(value), output);
		}


		void toStringPretty(u64 value, NSpan<char> output)
		{
			char* c = output.begin();
			char* num_start = output.begin();
			u32 length = output.length();
			if (length > 0)
			{
				if (value == 0)
				{
					if (length == 1)
					{
						return;
					}
					*c = '0';
					*(c + 1) = 0;
					return;
				}
				int counter = 0;
				while (value > 0 && length > 1)
				{
					*c = value % 10 + '0';
					value = value / 10;
					--length;
					++c;
					if ((counter + 1) % 3 == 0 && length > 1 && value > 0)
					{
						*c = ',';
						++c;
						counter = 0;
					}
					else
					{
						++counter;
					}
				}
				if (length > 0)
				{
					reverse(num_start, (int)(c - num_start));
					*c = 0;
					return;
				}
			}
			return;
		}


		char* toString(i32 value, NSpan<char> output)
		{
			char* c = output.begin();
			u32 length = output.length();
			if (length < 2) return nullptr;

			if (value < 0) {
				value = -value;
				--length;
				c[0] = '-';
				c[1] = 0;
				++c;
			}
			return toString((u32)value, NSpan(c, length));
		}


		char* toString(i64 value, NSpan<char> output)
		{
			char* c = output.begin();
			u32 length = output.length();
			if (length < 2) return nullptr;
			if (value < 0) {
				value = -value;
				--length;
				c[0] = '-';
				c[1] = 0;
				++c;
			}
			return toString((u64)value, NSpan(c, length));
		}


		char* toString(u64 value, NSpan<char> output)
		{
			char* c = output.begin();
			char* num_start = output.begin();
			u32 length = output.length();
			if (length < 2) return nullptr;

			if (value == 0) {
				*c = '0';
				*(c + 1) = 0;
				return c + 1;
			}

			while (value > 0 && length > 0) {
				*c = value % 10 + '0';
				value = value / 10;
				--length;
				++c;
			}

			if (length > 0) {
				reverse(num_start, (int)(c - num_start));
				*c = 0;
				return c;
			}

			*(c - 1) = 0;
			return nullptr;
		}


		void toStringHex(u8 value, NSpan<char> output)
		{
			//FIX Hex output format error
			if (output.length() < 3) {  // 2 char + '\0'
				return;
			}
			u8 first = value / 16;
			output[0] = (first > 9) ? ('A' + first - 10) : ('0' + first);

			u8 second = value % 16;
			output[1] = (second > 9) ? ('A' + second - 10) : ('0' + second);

			output[2] = '\0'; // end string
		}


		char* toString(u32 value, NSpan<char> output)
		{
			char* c = output.begin();
			char* num_start = output.begin();
			u32 length = output.length();
			if (length < 2) return nullptr;

			if (value == 0) {
				*c = '0';
				*(c + 1) = 0;
				return c + 1;
			}

			while (value > 0 && length > 0) {
				*c = value % 10 + '0';
				value = value / 10;
				--length;
				++c;
			}
			if (length > 0) {
				reverse(num_start, (int)(c - num_start));
				*c = 0;
				return c;
			}
			*(c - 1) = 0;
			return nullptr;
		}


		NString toString(i32 value)
		{
			char val[11];
			NSpan<char> outVal{ val };
			char* end = toString(value, outVal);
			if (end == nullptr) {
				return NString::Empty;
			}
			return NString{ &val[0], end };
		}


		NString toString(i64 value)
		{
			char val[21];
			NSpan<char> outVal{ val };
			char* end = toString(value, outVal);
			if (end == nullptr) {
				return NString::Empty;
			}
			return NString{ &val[0], end };
		}


		NString toString(u32 value)
		{
			char val[20];
			NSpan<char> outVal{ val };
			char* end = toString(value, outVal);
			if (end == nullptr) {
				return NString::Empty;
			}
			return NString{ &val[0], end };
		}

		NString toString(u64 value)
		{
			char val[21];
			NSpan<char> outVal{ val };
			char* end = toString(value, outVal);
			if (end == nullptr) {
				return NString::Empty;
			}
			return NString{ &val[0], end };
			return NString::format("{}", value);
		}


		static int getDecimalDigitCount(float frac, int maxDigits = 7)
		{
			frac = fabsf(frac);
			int count = 0;
			for (int i = 0; i < maxDigits; ++i) {
				frac *= 10.0f;
				int digit = (int)(frac + 1e-4f);  // add little bias to avoid conflict
				frac -= digit;

				++count;

				// check ends
				if (fabsf(frac) < 1e-4f) {
					break;
				}
			}
			return count;
		}


		static int getDecimalDigitCountD(double frac, int maxDigits = 17)
		{
			frac = std::fabs(frac);
			int count = 0;
			int lastNonZeroDigit = 0;

			for (int i = 0; i < maxDigits; ++i) {
				frac *= 10.0;
				int digit = (int)(frac + 1e-12);
				frac -= digit;

				++count;
				if (digit != 0) lastNonZeroDigit = count;

				if (std::fabs(frac) < 1e-12) {
					break;
				}
			}
			return lastNonZeroDigit;
		}


		NString toString(f32 value)
		{
			char val[21];
			NSpan<char> outVal{ val };
			auto after_point = getDecimalDigitCount(value - floor(value));
			char* end = toString((f32)value, outVal, after_point);
			if (end == nullptr) {
				return NString::Empty;
			}
			return NString{ &val[0], end };
			return NString::format("{}", value);
		}


		NString toString(f64 value)
		{
			char val[21];
			NSpan<char> outVal{ val };
			auto after_point = getDecimalDigitCountD(value - floor(value));
			char* end = toString((f32)value, outVal, after_point);
			if (end == nullptr) {
				return NString::Empty;
			}
			return NString{ &val[0], end };
			return NString::format("{}", value);
		}


		// returns new end or null if failed
		static char* increment(const char* output, char* end, bool is_space_after)
		{
			char carry = 1;
			{
				char* c = end - 1; // skip '\0'
				while (c >= output)
				{
					if (*c == '.')
					{
						--c;
					}
					*c += carry;
					if (*c > '9')
					{
						*c = '0';
						carry = 1;
					}
					else
					{
						carry = 0;
						break;
					}
					--c;
				}
			}
			if (carry && is_space_after) {
				char* c = end; // including '\0' at the end of the String
				while (c >= output) {
					*(c + 1) = *c;
					--c;
				}
				++c;
				*c = '1';
				return end + 1;
			}
			if (carry) return nullptr;
			return end;
		}


		char* toString(f32 value, NSpan<char> output, int after_point)
		{
			return toString(double(value), output, after_point);
		}


		char* toString(bool value, NSpan<char> output)
		{
			if (output.length() < (value ? 5 : 6)) // FIX: "true" need 5 bytes（include '\0'），"false" need 6
				return nullptr;

			NStringView str = value ? "true" : "false";
			return copyString(output, str);
		}


		char* toString(f64 value, NSpan<char> out, int after_point)
		{
			char* output = out.begin();
			u32 length = out.length();
			if (length < 2) return nullptr;

			if (value < 0) {
				*output = '-';
				++output;
				value = -value;
				--length;
			}
			// int part
			int exponent = value == 0 ? 0 : (int)log10(value);
			double num = value;
			char* c = output;
			if (num < 1 && length > 1) {
				*c = '0';
				++c;
				--length;
			}
			else {
				while ((num >= 1 || exponent >= 0) && length > 1) {
					const double power = pow(10.0, (double)exponent);
					char digit = (char)floor(num / power);
					num -= digit * power;
					*c = digit + '0';
					--exponent;
					--length;
					++c;
				}
			}
			// decimal part
			double dec_part = num;
			if (length > 1 && after_point > 0) {
				*c = '.';
				++c;
				--length;
			}
			else if (length > 0 && after_point == 0) {
				*c = 0;
				return c;
			}
			else {
				return nullptr;
			}
			while (length > 1 && after_point > 0) {
				dec_part *= 10;
				char tmp = (char)dec_part;
				*c = tmp + '0';
				dec_part -= tmp;
				++c;
				--length;
				--after_point;
			}
			*c = 0;
			if ((int)(dec_part + 0.5f))
				c = increment(output, c, length > 1);
			return c;
		}


		bool startsWith(NStringView str, NStringView prefix)
		{
			if (str.length() < prefix.length())
				return false;
			NStringView pfx(str.begin(), prefix.length());
			return equalStrings(pfx, prefix);
		}


		bool startsWithInsensitive(NStringView str, NStringView prefix)
		{
			if (str.length() < prefix.length())
				return false;
			NStringView pfx(str.begin(), prefix.length());
			return equalIStrings(pfx, prefix);
		}


		char toLower(char c) {
			if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
			return c;
		}

		bool isWhitespace(char c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; }
		bool isUpper(char c) { return isupper(c) != 0; }
		bool isLower(char c) { return islower(c) != 0; }
		bool isAlpha(char c) { return iswalpha(c) != 0; }
		bool isPunct(char c) { return ispunct(c) != 0; }
		bool isAlnum(char c) { return isalnum(c) != 0; }
		bool isDigit(char c) { return isdigit(c) != 0; }
		bool isHexDigit(char c) { return isxdigit(c) != 0; }
		char toUpper(char c) { return toupper(c); }

		void fastConcat(char* d, const char* a, const char* b)
		{
			size_t len1 = strlen(a);
			size_t len2 = strlen(b);

			if (len1 + len2 + 1 >= strlen(d)) {
				return;
			}

			strcpy(d, a);
			strcpy(d + len1, a);
			d[len1 + len2] = '\0';
		}


		bool fromHexString(const char* str, i32 length, u32& result)
		{
			result = 0;

			u32 sum = 0;
			const char* p = str;
			const char* end = str + length;
			if (*p == '0' && *(p + 1) == 'x')
				p += 2;
			while (*p && p < end)
			{
				i32 c = *p - '0';
				if (c < 0 || c > 9)
				{
					c = toLower(*p) - 'a' + 10;
					if (c < 10 || c > 15)
						return false;
				}
				sum = 16 * sum + c;
				p++;
			}
			result = sum;
			return true;
		}


		NString skipZeroToZero(const char* str)
		{
			const char* end = str;
			while (*end != '\0') {
				end++;
				if (*end == '\0')
					end++;
			}
			return NString{ str, (i32)(end - str) };
		}


		u32 getStringsHash(const char* str)
		{
			u32 hash = 5381;
			char c;
			if (str)
			{
				while ((c = *str++) != 0)
					hash = ((hash << 5) + hash) + (u32)c;
			}
			return hash;
		}


		u32 getStringsHash(const char* str, i32 length)
		{
			u32 hash = 5381;
			char c;
			if (str)
			{
				while ((c = *str++) != 0 && length-- > 0)
					hash = ((hash << 5) + hash) + (u32)c;
			}
			return hash;
		}
	}

    void replaceAll(std::string& str, const char* from, const char* to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, strlen(from), to);
            start_pos += strlen(to);
        }
    }

    void splitStr(std::vector<std::string>& out, const std::string& str, char delim, bool skipEmpty) {
        size_t start = 0;
        size_t pos = 0;

        while ((pos = str.find(delim, start)) != std::string::npos) {
            if (pos > start || !skipEmpty) {
                out.emplace_back(str.substr(start, pos - start));
            }
            start = pos + 1;
        }

        if (start < str.size() || !skipEmpty) {
            out.emplace_back(str.substr(start));
        }
    }

    void splitStr(std::vector<std::string>& out, const std::string& str, const char* delim, bool skipEmpty) {
        size_t start = 0;
        size_t pos = 0;
        size_t dlen = std::strlen(delim);

        while ((pos = str.find(delim, start)) != std::string::npos) {
            if (pos > start || !skipEmpty) {
                out.emplace_back(str.substr(start, pos - start));
            }
            start = pos + dlen;
        }

        if (start < str.size() || !skipEmpty) {
            out.emplace_back(str.substr(start));
        }
    }

    void splitAt(std::vector<std::string>& out, int pos, const std::string& str) {
        out.clear();
        if (pos < 0 || pos > static_cast<int>(str.size())) {
            out.push_back(str);
            return;
        }
        out.push_back(str.substr(0, pos));
        out.push_back(str.substr(pos));
    }

    int findLast(const std::string& str, char hint) {
        auto pos = str.find_last_of(hint);
        if (pos == std::string::npos) return -1;
        return static_cast<int>(pos);
    }
}