/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/base/Assert.hpp>
#include <nbase/memory/Allocation.hpp>
#include <nbase/base/Format.hpp>
#include <nbase/utils/StringUtils.hpp>

namespace neo {

	template<typename T, typename AllocationType>
	class NArray;
	class NStringView;

	// Represents text as a sequence of UTF8 characters. Container uses a single dynamic memory allocation to store the characters data. wcharacters sequence is always null-terminated.
	class NString final
	{
	public:
		// Instance of the empty string.
		static NString Empty;

	public:
		NString() = default;
		NString(const NString& str)
		{
			set(str.get(), str.length());
		}
		NString(NString&& str) noexcept
		{
			m_data = str.m_data;
			m_length = str.m_length;
			str.m_data = nullptr;
			str.m_length = 0;
		}
		NString(const char* str)
			: NString(str, strings::length(str))
		{
		}
		NString(const char* start, const char* end)
			: NString(start, static_cast<i32>(end - start))
		{
		}
		explicit NString(const char* str, i32 length)
		{
			set(str, length);
		}
		NString(const NStringView& str);


		~NString()
		{
			neo::free(m_data);
		}

	public:
		// Clears this instance. Frees the memory and sets the string to empty.
		void clear()
		{
			neo::free(m_data);
			m_data = nullptr;
			m_length = 0;
		}

	public:
		// Gets the character at the specific index.
		NE_FORCE_INLINE char& operator[](i32 index)
		{
			NE_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}


		// Gets the character at the specific index.
		NE_FORCE_INLINE const char& operator[](i32 index) const
		{
			NE_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}

	public:
		bool startsWith(char c, bool searchCase = true) const;
		bool endsWith(char c, bool searchCase = true) const;

		bool startsWith(const NString& prefix, bool searchCase = true) const;
		bool endsWith(const NString& suffix, bool searchCase = true) const;

		bool startsWith(const NStringView& prefix, bool searchCase = true) const;
		bool endsWith(const NStringView& suffix, bool searchCase = true) const;

		i32 replace(char searchwchar, char replacementwchar, bool searchCase = true);
		i32 replace(const char* searchText, const char* replacementText, bool searchCase = true);
		i32 replace(const char* searchText, i32 searchTextLength, const char* replacementText, i32 replacementTextLength, bool searchCase = true);

		void reverse();
		void resize(i32 length);

		// Lexicographically tests how this string compares to the other given string.
		// In case sensitive mode 'A' is less than 'a'.
		i32 compare(const NString& str, bool searchCase = true) const;

	public:
		// Returns true if string is empty.
		NE_FORCE_INLINE bool isEmpty() const
		{
			return m_length == 0;
		}

		// Returns true if string isn't empty.
		NE_FORCE_INLINE bool hasChars() const
		{
			return m_length != 0;
		}

		// Gets the length of the string.
		NE_FORCE_INLINE i32 length() const
		{
			return m_length;
		}

		NE_FORCE_INLINE const char* operator*() const
		{
			return m_data;
		}
		NE_FORCE_INLINE char* operator*()
		{
			return m_data;
		}
		NE_FORCE_INLINE char* get()
		{
			return m_data;
		}
		NE_FORCE_INLINE const char* get() const
		{
			return m_data;
		}
		NE_FORCE_INLINE const char* getText() const
		{
			return m_data ? m_data : (const char*)"";
		}

	public:
		NE_FORCE_INLINE bool contains(const char* subStr, bool searchCase = true) const
		{
			return find(subStr, searchCase) != -1;
		}
		NE_FORCE_INLINE bool contains(const NString& subStr, bool searchCase = true) const
		{
			return find(*subStr, searchCase) != -1;
		}

		i32 find(char c) const;
		i32 findLast(char c) const;

		i32 find(const char* subStr, bool searchCase = true, i32 startPosition = -1) const;
		i32 findLast(const char* subStr, bool searchCase = true, i32 startPosition = -1) const;

		NE_FORCE_INLINE i32 find(const NString& subStr, bool searchCase = true, i32 startPosition = -1) const
		{
			return find(subStr.get(), searchCase, startPosition);
		}
		NE_FORCE_INLINE i32 findLast(const NString& subStr, bool searchCase = true, i32 startPosition = -1) const
		{
			return findLast(subStr.get(), searchCase, startPosition);
		}

		i32 findFirstOf(char c, i32 startPos = 0) const;
		i32 findFirstOf(const char* str, i32 startPos = 0) const;

		void reserve(i32 length);

	public:
		// Sets an array of characters to the string.
		void set(const char* chars, i32 length);

		// Appends an array of characters to the string.
		void append(const char* chars, i32 count);
		// Appends the specified text to this string.
		NE_FORCE_INLINE NString& append(const char* text)
		{
			append(&text[0], strlen(text));
			return *this;
		}
		// Appends the specified text to this string.
		NE_FORCE_INLINE NString& append(const NString& text)
		{
			append(text.get(), text.length());
			return *this;
		}
		// Appends the specified text to this string.
		NE_FORCE_INLINE NString& append(const std::string& text)
		{
			append(&text[0], text.length());
			return *this;
		}
		// Appends the specified text to this string.
		NE_FORCE_INLINE NString& append(const std::string_view text)
		{
			append(&text[0], text.length());
			return *this;
		}
		// Appends the specified character to this string.
		NE_FORCE_INLINE NString& append(const char c)
		{
			NE_ASSERT(c != 0);
			append(&c, 1);
			return *this;
		}


		// Appends the specified text to this string
		NE_FORCE_INLINE NString& operator+=(const char* str)
		{
			append(str, strings::length(str));
			return *this;
		}
		// Appends the specified character to this string.
		NString& operator+=(const char c)
		{
			NE_ASSERT(c != 0);
			append(&c, 1);
			return *this;
		}
		// Appends the specified text to this string.
		NE_FORCE_INLINE NString& operator+=(const NString& str)
		{
			append(str.get(), str.length());
			return *this;
		}


		// Appends the specified text to this string.
		NString& operator+=(const NStringView& str);
		// Concatenates a string with a character.
		friend NString operator+(const NString& a, const char b)
		{
			NString result;
			result.m_length = a.length() + 1;
			result.m_data = (char*)neo::allocAligned((result.m_length + 1) * sizeof(char), 16);
			memcpy(result.m_data, a.get(), a.length() * sizeof(char));
			result.m_data[a.length()] = b;
			result.m_data[result.m_length] = 0;
			return result;
		}

	public:
		NE_FORCE_INLINE NString& operator=(NString&& s) noexcept
		{
			if (this != &s)
			{
				neo::free(m_data);
				m_data = s.m_data;
				m_length = s.m_length;
				s.m_data = nullptr;
				s.m_length = 0;
			}
			return *this;
		}
		NE_FORCE_INLINE NString& operator=(const NString& s)
		{
			if (this != &s)
				set(s.get(), s.length());
			return *this;
		}
		NString& operator=(const NStringView& s);
		NString& operator=(const char* str)
		{
			if (m_data != str)
				set(str, strings::length(str));
			return *this;
		}
		NString& operator=(const char c)
		{
			set(&c, 1);
			return *this;
		}

	public:
		NE_FORCE_INLINE friend NString operator+(const NString& a, const NString& b)
		{
			return concatNStrings<const NString&, const NString&>(a, b);
		}
		NE_FORCE_INLINE friend NString operator+(NString&& a, const NString& b)
		{
			return concatNStrings(moveTemp(a), b);
		}
		NE_FORCE_INLINE friend NString operator+(const NString& a, NString&& b)
		{
			return concatNStrings<const NString&, NString&&>(a, moveTemp(b));
		}
		NE_FORCE_INLINE friend NString operator+(NString&& a, NString&& b)
		{
			return concatNStrings<NString&&, NString&&>(moveTemp(a), moveTemp(b));
		}
		NE_FORCE_INLINE friend NString operator+(const char* a, const NString& b)
		{
			return concatwcharstoString<const NString&>(a, b);
		}
		NE_FORCE_INLINE friend NString operator+(const char* a, NString&& b)
		{
			return concatwcharstoString<NString&&>(a, moveTemp(b));
		}
		NE_FORCE_INLINE friend NString operator+(const NString& a, const char* b)
		{
			return concatNStringTowchars<const NString&>(a, b);
		}
		NE_FORCE_INLINE friend NString operator+(NString&& a, const char* b)
		{
			return concatNStringTowchars<NString&&>(moveTemp(a), b);
		}
		NE_FORCE_INLINE bool operator<=(const char* other) const
		{
			return strings::compareStrings(this->getText(), other) <= 0;
		}
		NE_FORCE_INLINE bool operator<(const char* other) const
		{
			return strings::compareStrings(this->getText(), other) < 0;
		}
		NE_FORCE_INLINE bool operator<(const NString& other) const
		{
			return strings::compareStrings(this->getText(), other.getText()) < 0;
		}
		NE_FORCE_INLINE bool operator>=(const char* other) const
		{
			return strings::compareStrings(this->getText(), other) >= 0;
		}
		NE_FORCE_INLINE bool operator>(const char* other) const
		{
			return strings::compareStrings(this->getText(), other) > 0;
		}
		NE_FORCE_INLINE bool operator>(const NString& other) const
		{
			return strings::compareStrings(this->getText(), other.getText()) > 0;
		}
		NE_FORCE_INLINE bool operator==(const char* other) const
		{
			return strings::compareStrings(this->getText(), other) == 0;
		}
		NE_FORCE_INLINE bool operator==(const NString& other) const
		{
			return strings::compareStrings(this->getText(), other.getText()) == 0;
		}
		NE_FORCE_INLINE bool operator!=(const char* other) const
		{
			return strings::compareStrings(this->getText(), other) != 0;
		}
		NE_FORCE_INLINE bool operator!=(const NString& other) const
		{
			return strings::compareStrings(this->getText(), other.getText()) != 0;
		}
		NE_FORCE_INLINE bool operator<=(const NString& other) const
		{
			return strings::compareStrings(this->getText(), other.getText()) <= 0;
		}
		NE_FORCE_INLINE bool operator>=(const NString& other) const
		{
			return strings::compareStrings(this->getText(), other.getText()) >= 0;
		}

	public:
		NString& operator/=(const char* str);
		NString& operator/=(char c);
		NE_FORCE_INLINE NString& operator/=(const NString& str)
		{
			return operator/=(*str);
		}
		NString& operator/=(const NStringView& str);
		NE_FORCE_INLINE NString operator/(const char* str) const
		{
			return NString(*this) /= str;
		}
		NE_FORCE_INLINE NString operator/(const char c) const
		{
			return NString(*this) /= c;
		}
		NE_FORCE_INLINE NString operator/(const NString& str) const
		{
			return NString(*this) /= str;
		}
		NE_FORCE_INLINE NString operator/(const NStringView& str) const
		{
			return NString(*this) /= str;
		}

	public:
		NString toLower() const;
		NString toUpper() const;


		NE_FORCE_INLINE NString left(i32 count) const
		{
			const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
			return NString(**this, countClamped);
		}
		NE_FORCE_INLINE NString right(i32 count) const
		{
			const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
			return NString(**this + length() - countClamped);
		}

		NString substring(i32 startIndex) const
		{
			NE_ASSERT(startIndex >= 0 && startIndex < length());
			return NString(m_data + startIndex, m_length - startIndex);
		}
		NString substring(i32 startIndex, i32 count) const
		{
			NE_ASSERT(startIndex >= 0 && startIndex + count <= length() && count >= 0);
			return NString(m_data + startIndex, count);
		}


		// Inserts string into current string instance at given location.
		void insert(i32 startIndex, const NString& other);
		// Removes characters from the string at given location and length.
		void remove(i32 startIndex, i32 length);
		// Splits a string into substrings that are based on the character.
		void split(char c, NArray<NString, HeapAllocation>& results) const;

	public:
		// Formats the message and gets it as a string.
		template<typename... Args>
		static NString format(const char* format, const Args& ... args)
		{
			neo::StlAllocator<char> allocator;
			neo::fmt_memory_buffer buf{allocator};
			neo::format(buf, format, args...);
			return NString(buf.data(), (i32)buf.size());
		}

	public:
		NString toString() const
		{
			return NString(get(), length());
		}

	private:
		template<typename T1, typename T2>
		static NString concatNStrings(T1 left, T2 right)
		{
			if (left.isEmpty())
				return moveTemp(right);
			if (right.isEmpty())
				return moveTemp(left);

			const char* leftStr = left.get();
			const i32 leftLen = left.length();
			const char* rightStr = right.get();
			const i32 rightLen = right.length();

			NString result;
			result.reserve(leftLen + rightLen);
			memcpy(result.get(), leftStr, leftLen * sizeof(char));
			memcpy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}


		template<typename T>
		static NString concatwcharstoString(const char* left, T right)
		{
			if (!left || !*left)
				return moveTemp(right);

			const char* leftStr = left;
			const i32 leftLen = strings::length(left);
			const char* rightStr = right.get();
			const i32 rightLen = right.length();

			NString result;
			result.reserve(leftLen + rightLen);
			memcpy(result.get(), leftStr, leftLen * sizeof(char));
			memcpy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}


		template<typename T>
		static NString concatNStringTowchars(T left, const char* right)
		{
			if (!right || !*right)
				return moveTemp(left);

			const char* leftStr = left.get();
			const i32 leftLen = left.length();
			const char* rightStr = right;
			const i32 rightLen = strings::length(right);

			NString result;
			result.reserve(leftLen + rightLen);
			memcpy(result.get(), leftStr, leftLen * sizeof(char));
			memcpy(result.get() + leftLen, rightStr, rightLen * sizeof(char));

			return result;
		}


	private:
		char* m_data = nullptr;
		i32 m_length = 0;
	};


	inline u32 getHash(const NString& key)
	{
		return strings::getStringsHash(key.get());
	}
}


template<>
struct std::formatter<neo::NString, char>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return ctx.begin();
	}
	template<typename FormatContext>
	auto format(const neo::NString& v, FormatContext& ctx) const {
		return std::copy_n(v.get(), v.length(), ctx.out());
	}
};
