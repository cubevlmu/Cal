/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/base/Assert.hpp>
#include <nbase/utils/StringUtils.hpp>
#include <format>

namespace neo {

	// Represents static text view as a sequence of ANSI characters. wcharacters sequence might not be null-terminated
	class NStringView final
	{
	public:
		static NStringView Empty;

	public:
		NStringView(const class NString& str);
		NStringView(const char* str)
		{
			m_data = str;
			m_length = strings::length(str);
		}
		constexpr NStringView()
			: m_data(nullptr)
			, m_length(0)
		{
		}
		constexpr NStringView(const char* data, i32 length)
			: m_data(data)
			, m_length(length)
		{
		}
		constexpr NStringView(const NStringView& other)
			: m_data(other.m_data)
			, m_length(other.m_length)
		{
		}


	public:
		NE_FORCE_INLINE NStringView& operator=(const char* str)
		{
			m_data = str;
			m_length = strings::length(str);
			return *this;
		}
		NE_FORCE_INLINE bool operator==(const NStringView& other) const
		{
			return m_length == other.m_length && (m_length == 0 || strings::compareStrings(m_data, other.m_data, m_length) == 0);
		}
		NE_FORCE_INLINE bool operator!=(const NStringView& other) const
		{
			return !(*this == other);
		}
		NE_FORCE_INLINE bool operator==(const char* other) const
		{
			return *this == NStringView(other);
		}
		NE_FORCE_INLINE bool operator!=(const char* other) const
		{
			return !(*this == NStringView(other));
		}
		bool operator==(const NString& other) const;
		bool operator!=(const NString& other) const;

	public:
		NE_FORCE_INLINE const char& operator[](i32 index) const
		{
			NE_ASSERT(index >= 0 && index <= m_length);
			return m_data[index];
		}

		NE_FORCE_INLINE NStringView& operator=(const NStringView& other)
		{
			if (this != &other)
			{
				m_data = other.m_data;
				m_length = other.m_length;
			}
			return *this;
		}

		i32 compare(const NStringView& str, bool searchCase = true) const;

	public:
		NE_FORCE_INLINE bool isEmpty() const
		{
			return m_length == 0;
		}

		NE_FORCE_INLINE bool hasChars() const
		{
			return m_length != 0;
		}

		NE_FORCE_INLINE constexpr i32 length() const
		{
			return m_length;
		}

		NE_FORCE_INLINE constexpr const char* operator*() const
		{
			return m_data;
		}

		NE_FORCE_INLINE constexpr const char* get() const
		{
			return m_data;
		}

		NE_FORCE_INLINE const char* getText() const
		{
			return m_data ? m_data : (const char*)"";
		}

		NE_FORCE_INLINE const char* begin() const {
			return m_data ? m_data : "";
		}
		NE_FORCE_INLINE const char* end() const {
			return m_data ? m_data + m_length * sizeof(char) : "";
		}

	public:
		// Searches the string for the occurrence of a character.
		i32 find(char c) const;
		// Searches the string for the last occurrence of a character.
		i32 findLast(char c) const;

		bool startsWith(char c, bool searchCase = false) const;
		bool endsWith(char c, bool searchCase = false) const;

		bool startsWith(const NStringView& prefix, bool searchCase = false) const;
		bool endsWith(const NStringView& suffix, bool searchCase = false) const;

	public:
		// Gets the left most given number of characters.
		NStringView left(i32 count) const;
		// Gets the string of characters from the right (end of the string).
		NStringView right(i32 count) const;
		// Retrieves substring created from characters starting from startIndex to the NString end.
		NStringView substring(i32 startIndex) const;
		// Retrieves substring created from characters starting from start index.
		NStringView substring(i32 startIndex, i32 count) const;

	public:
		NString toString() const;

	private:
		const char* m_data;
		i32 m_length;
	};


	inline u32 getHash(const NStringView& key)
	{
		return strings::getStringsHash(key.get(), key.length());
	}

	bool operator==(const NString& a, const NStringView& b);
	bool operator!=(const NString& a, const NStringView& b);
}


template<>
struct std::formatter<neo::NStringView, char>
{
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return ctx.begin();
	}
	template<typename FormatContext>
	auto format(const neo::NStringView& v, FormatContext& ctx) const {
		return std::copy_n(v.get(), v.length(), ctx.out());
	}
};
