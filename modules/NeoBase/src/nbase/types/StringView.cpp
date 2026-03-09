/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "StringView.hpp"

#include "String.hpp"

namespace neo {

	NStringView NStringView::Empty;


	NStringView::NStringView(const NString& str)
		: NStringView(str.get(), str.length())
	{
	}


	bool NStringView::operator==(const NString& other) const
	{
		return this->compare(NStringView(other)) == 0;
	}


	bool NStringView::operator!=(const NString& other) const
	{
		return this->compare(NStringView(other)) != 0;
	}


	NStringView NStringView::left(i32 count) const
	{
		const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
		return NStringView(**this, countClamped);
	}


	NStringView NStringView::right(i32 count) const
	{
		const i32 countClamped = count < 0 ? 0 : count < length() ? count : length();
		return NStringView(**this + countClamped, length() - countClamped);
	}


	i32 NStringView::compare(const NStringView& str, bool searchCase) const
	{
		const bool thisIsShorter = NStringView::length() < str.length();
		const i32 minLength = thisIsShorter ? NStringView::length() : str.length();
		const i32 prefixCompare = searchCase ? strings::compareStrings(getText(), str.getText(), minLength) : strings::compareIStrings(getText(), str.getText(), minLength);
		if (prefixCompare != 0)
			return prefixCompare;
		if (NStringView::length() == str.length())
			return 0;
		return thisIsShorter ? -1 : 1;
	}


	i32 NStringView::find(char c) const
	{
		const char* NE_RESTRICT start = get();
		for (const char* NE_RESTRICT data = start, *NE_RESTRICT dataEnd = data + m_length; data != dataEnd; ++data)
		{
			if (*data == c)
			{
				return static_cast<i32>(data - start);
			}
		}
		return -1;
	}


	i32 NStringView::findLast(char c) const
	{
		const char* NE_RESTRICT end = get() + m_length;
		for (const char* NE_RESTRICT data = end, *NE_RESTRICT dataStart = data - m_length; data != dataStart;)
		{
			--data;
			if (*data == c)
			{
				return static_cast<i32>(data - dataStart);
			}
		}
		return -1;
	}


	bool NStringView::startsWith(char c, bool searchCase) const
	{
		const i32 length = NStringView::length();
		if (!searchCase)
			return length > 0 && strings::toLower(m_data[0]) == strings::toLower(c);
		return length > 0 && m_data[0] == c;
	}


	bool NStringView::endsWith(char c, bool searchCase) const
	{
		const i32 length = NStringView::length();
		if (!searchCase)
			return length > 0 && strings::toLower(m_data[length - 1]) == strings::toLower(c);
		return length > 0 && m_data[length - 1] == c;
	}


	bool NStringView::startsWith(const NStringView& prefix, bool searchCase) const
	{
		if (prefix.isEmpty() || NStringView::length() < prefix.length())
			return false;
		// We know that this NStringView is not empty, and therefore Get() below is valid.
		if (searchCase == false)
			return strings::compareIStrings(this->get(), *prefix, prefix.length()) == 0;
		return strings::compareStrings(this->get(), *prefix, prefix.length()) == 0;
	}


	bool NStringView::endsWith(const NStringView& suffix, bool searchCase) const
	{
		if (suffix.isEmpty() || NStringView::length() < suffix.length())
			return false;
		// We know that this NStringView is not empty, and therefore accessing data below is valid.
		if (searchCase == false)
			return strings::compareIStrings(&(*this)[length() - suffix.length()], *suffix) == 0;
		return strings::compareStrings(&(*this)[length() - suffix.length()], *suffix) == 0;
	}


	NStringView NStringView::substring(i32 startIndex) const
	{
		NE_ASSERT(startIndex >= 0 && startIndex < length());
		return {get() + startIndex, length() - startIndex};
	}


	NStringView NStringView::substring(i32 startIndex, i32 count) const
	{
		NE_ASSERT(startIndex >= 0 && startIndex + count <= length() && count >= 0);
		return {get() + startIndex, count };
	}


	NString NStringView::toString() const
	{
		return NString(m_data, m_length);
	}


	bool operator==(const NString& a, const NStringView& b)
	{
		return a.length() == b.length() && strings::compareStrings(a.getText(), b.getText(), b.length()) == 0;
	}


	bool operator!=(const NString& a, const NStringView& b)
	{
		return a.length() != b.length() || strings::compareStrings(a.getText(), b.getText(), b.length()) != 0;
	}
}