/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "String.hpp"

#include "StringView.hpp"
#include "Array.hpp"

namespace neo {

	NString NString::Empty{};
	NString::NString(const NStringView& str)
	{
		set(str.get(), str.length());
	}


	NString& NString::operator/=(const char* str)
	{
		const i32 length = m_length;
		if (length > 0 && m_data[length - 1] != '/' && m_data[length - 1] != '\\'
			&& (str == nullptr || (str[0] != '/' && str[0] != '\\')))
		{
			*this += '/';
		}
		return *this += str;
	}


	NString& NString::operator/=(const char c)
	{
		const i32 length = m_length;
		if (length > 0 && m_data[length - 1] != '/' && m_data[length - 1] != '\\')
		{
			*this += '/';
		}
		return *this += c;
	}


	NString& NString::operator/=(const NStringView& str)
	{
		const i32 length = m_length;
		if (length > 1 && m_data[length - 1] != '/' && m_data[length - 1] != '\\'
			&& (str == nullptr || (str[0] != '/' && str[0] != '\\')))
		{
			*this += '/';
		}
		return *this += str;
	}


	bool NString::startsWith(char c, bool searchCase) const
	{
		const i32 length = NString::length();
		if (searchCase == true)
			return length > 0 && m_data[0] == c;
		return length > 0 && strings::toLower(m_data[0]) == strings::toLower(c);
	}


	bool NString::endsWith(char c, bool searchCase) const
	{
		const i32 length = NString::length();
		if (searchCase == true)
			return length > 0 && m_data[length - 1] == c;
		return length > 0 && strings::toLower(m_data[length - 1]) == strings::toLower(c);
	}


	bool NString::startsWith(const NString& prefix, bool searchCase) const
	{
		if (prefix.isEmpty())
			return true;
		if (length() < prefix.length())
			return false;
		if (searchCase == false)
			return strings::compareIStrings(this->getText(), *prefix, prefix.length()) == 0;
		return strings::compareStrings(this->getText(), *prefix, prefix.length()) == 0;
	}


	bool NString::endsWith(const NString& suffix, bool searchCase) const
	{
		if (suffix.isEmpty())
			return true;
		if (length() < suffix.length())
			return false;
		if (searchCase == false)
			return strings::compareIStrings(&(*this)[NString::length() - suffix.length()], *suffix) == 0;
		return strings::compareStrings(&(*this)[length() - suffix.length()], *suffix) == 0;
	}


	i32 NString::replace(char searchwchar, char replacementwchar, bool searchCase)
	{
		i32 replacedwchars = 0;
		const i32 length = NString::length();
		if (searchCase == false)
		{
			const char toCompare = strings::toLower(searchwchar);
			for (i32 i = 0; i < length; i++)
			{
				if (strings::toLower(m_data[i]) == toCompare)
				{
					m_data[i] = replacementwchar;
					replacedwchars++;
				}
			}
		}
		else
		{
			for (i32 i = 0; i < length; i++)
			{
				if (m_data[i] == searchwchar)
				{
					m_data[i] = replacementwchar;
					replacedwchars++;
				}
			}
		}
		return replacedwchars;
	}


	i32 NString::replace(const char* searchText, const char* replacementText, bool searchCase)
	{
		const i32 searchTextLength = strings::length(searchText);
		const i32 replacementTextLength = strings::length(replacementText);
		return replace(searchText, searchTextLength, replacementText, replacementTextLength, searchCase);
	}


	i32 NString::replace(const char* searchText, i32 searchTextLength, const char* replacementText, i32 replacementTextLength, bool searchCase)
	{
		if (!hasChars() || searchTextLength == 0)
			return 0;

		i32 replacedCount = 0;

		if (searchTextLength == replacementTextLength)
		{
			char* pos = (char*)(searchCase == false ? strings::findInsensitive(m_data, searchText) : strings::find(m_data, searchText));
			while (pos != nullptr)
			{
				replacedCount++;

				for (i32 i = 0; i < replacementTextLength; i++)
					pos[i] = replacementText[i];

				if (pos + searchTextLength - **this < length())
					pos = (char*)(searchCase == false ? strings::findInsensitive(pos + searchTextLength, searchText) : strings::find(pos + searchTextLength, searchText));
				else
					break;
			}
		}
		else if (contains(searchText, searchCase))
		{
			char* readPosition = m_data;
			char* searchPosition = (char*)(searchCase == false ? strings::findInsensitive(readPosition, searchText) : strings::find(readPosition, searchText));
			while (searchPosition != nullptr)
			{
				replacedCount++;
				readPosition = searchPosition + searchTextLength;
				searchPosition = (char*)(searchCase == false ? strings::findInsensitive(readPosition, searchText) : strings::find(readPosition, searchText));
			}

			const auto oldLength = m_length;
			const auto oldData = m_data;
			m_length += replacedCount * (replacementTextLength - searchTextLength);
			m_data = (char*)neo::allocAligned((m_length + 1) * sizeof(char), 16);

			char* writePosition = m_data;
			readPosition = oldData;
			searchPosition = (char*)(searchCase == false ? strings::findInsensitive(readPosition, searchText) : strings::find(readPosition, searchText));
			while (searchPosition != nullptr)
			{
				const i32 writeOffset = (i32)(searchPosition - readPosition);
				memcpy(writePosition, readPosition, writeOffset * sizeof(char));
				writePosition += writeOffset;

				if (replacementTextLength > 0)
					memcpy(writePosition, replacementText, replacementTextLength * sizeof(char));
				writePosition += replacementTextLength;

				readPosition = searchPosition + searchTextLength;
				searchPosition = (char*)(searchCase == false ? strings::findInsensitive(readPosition, searchText) : strings::find(readPosition, searchText));
			}

			const i32 writeOffset = (i32)(oldData - readPosition) + oldLength;
			memcpy(writePosition, readPosition, writeOffset * sizeof(char));

			m_data[m_length] = 0;
			neo::free(oldData);
		}

		return replacedCount;
	}


	void NString::reverse()
	{
		char c;
		i32 tmp, count = m_length, end = count / 2;
		for (i32 i = 0; i < end; i++)
		{
			tmp = count - i - 2;
			c = m_data[i];
			m_data[i] = m_data[tmp];
			m_data[tmp] = c;
		}
	}


	void NString::resize(i32 length)
	{
		NE_ASSERT(length >= 0);
		if (m_length != length)
		{
			const auto oldData = m_data;
			const auto minLength = m_length < length ? m_length : length;
			m_length = length;
			m_data = (char*)neo::allocAligned((length + 1) * sizeof(char), 16);
			memcpy(m_data, oldData, minLength * sizeof(char));
			m_data[length] = 0;
			neo::free(oldData);
		}
	}


	i32 NString::compare(const NString& str, bool searchCase) const
	{
		if (searchCase == true)
			return strings::compareStrings(this->getText(), str.getText());
		return strings::compareIStrings(this->getText(), str.getText());
	}


	i32 NString::find(char c) const
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


	i32 NString::findLast(char c) const
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


	i32 NString::find(const char* subStr, bool searchCase, i32 startPosition) const
	{
		if (subStr == nullptr || !m_data)
			return -1;
		const char* start = m_data;
		if (startPosition != -1)
			start += startPosition < length() ? startPosition : length();
		const char* tmp = searchCase == false ? strings::findInsensitive(start, subStr) : strings::find(start, subStr);
		return tmp ? static_cast<i32>(tmp - **this) : -1;
	}


	i32 NString::findLast(const char* subStr, bool searchCase, i32 startPosition) const
	{
		const i32 subStrLen = strings::length(subStr);
		if (subStrLen == 0 || !m_data)
			return -1;
		if (startPosition == -1)
			startPosition = length();
		const char* start = m_data;
		if (searchCase == false)
		{
			for (i32 i = startPosition - subStrLen; i >= 0; i--)
			{
				if (strings::compareIStrings(start + i, subStr, subStrLen) == 0)
					return i;
			}
		}
		else
		{
			for (i32 i = startPosition - subStrLen; i >= 0; i--)
			{
				if (strings::compareStrings(start + i, subStr, subStrLen) == 0)
					return i;
			}
		}
		return -1;
	}


	i32 NString::findFirstOf(char c, i32 startPos) const
	{
		for (i32 i = startPos; i < length(); i++)
		{
			if (m_data[i] == c)
				return i;
		}
		return -1;
	}


	i32 NString::findFirstOf(const char* str, i32 startPos) const
	{
		if (!str)
			return -1;
		for (i32 i = startPos; i < m_length; i++)
		{
			const char c = m_data[i];
			const char* s = str;
			while (*s)
			{
				if (c == *s)
					return i;
				s++;
			}
		}
		return -1;
	}


	void NString::reserve(i32 length)
	{
		NE_ASSERT(length >= 0);
		if (length == m_length)
			return;
		neo::free(m_data);
		if (length != 0)
		{
			m_data = (char*)neo::allocAligned((length + 1) * sizeof(char), 16);
			m_data[length] = 0;
		}
		else
		{
			m_data = nullptr;
		}
		m_length = length;
	}


	void NString::set(const char* chars, i32 length)
	{
		NE_ASSERT(length >= 0);
		if (length == m_length)
		{
			if (m_data == chars)
				return;
			memcpy(m_data, chars, length * sizeof(char));
		}
		else
		{
			char* data = nullptr;
			if (length != 0)
			{
				data = (char*)neo::allocAligned((length + 1) * sizeof(char), 16);
				memcpy(data, chars, length * sizeof(char));
				data[length] = 0;
			}
			neo::free(m_data);
			m_data = data;
			m_length = length;
		}
	}


	void NString::append(const char* chars, i32 count)
	{
		if (count == 0)
			return;

		const auto oldData = m_data;
		const auto oldLength = m_length;

		m_length = oldLength + count;
		m_data = (char*)neo::allocAligned((m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, oldLength * sizeof(char));
		memcpy(m_data + oldLength, chars, count * sizeof(char));
		m_data[m_length] = 0;

		neo::free(oldData);
	}


	NString& NString::operator+=(const NStringView& str)
	{
		append(str.get(), str.length());
		return *this;
	}


	NString& NString::operator=(const NStringView& s)
	{
		set(s.get(), s.length());
		return *this;
	}


	bool NString::startsWith(const NStringView& prefix, bool searchCase) const
	{
		if (prefix.isEmpty() || prefix.length() > length())
			return false;
		if (searchCase == false)
			return !strings::compareIStrings(this->getText(), *prefix, prefix.length());
		return !strings::compareStrings(this->getText(), *prefix, prefix.length());
	}


	bool NString::endsWith(const NStringView& suffix, bool searchCase) const
	{
		if (suffix.isEmpty() || suffix.length() > length())
			return false;
		if (searchCase == false)
			return !strings::compareIStrings(&(*this)[length() - suffix.length()], *suffix);
		return !strings::compareStrings(&(*this)[length() - suffix.length()], *suffix);
	}


	NString NString::toLower() const
	{
		NString result(*this);
		for (i32 i = 0; i < result.length(); i++)
			result[i] = strings::toLower(result[i]);
		return result;
	}


	NString NString::toUpper() const
	{
		NString result(*this);
		for (i32 i = 0; i < result.length(); i++)
			result[i] = strings::toUpper(result[i]);
		return result;
	}


	void NString::insert(i32 startIndex, const NString& other)
	{
		NE_ASSERT(other.m_data != m_data);
		const i32 myLength = length();
		const i32 otherLength = other.length();
		NE_ASSERT(startIndex >= 0 && startIndex < m_length);

		if (otherLength == 0)
			return;

		if (myLength == 0)
		{
			operator=(other);
			return;
		}

		const auto oldData = m_data;
		const auto oldLength = m_length;

		m_length = oldLength + otherLength;
		m_data = (char*)neo::allocAligned((m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, startIndex * sizeof(char));
		memcpy(m_data + startIndex, other.get(), otherLength * sizeof(char));
		memcpy(m_data + startIndex + otherLength, oldData + startIndex, (oldLength - startIndex) * sizeof(char));
		m_data[m_length] = 0;

		neo::free(oldData);
	}


	void NString::remove(i32 startIndex, i32 length)
	{
		const auto oldData = m_data;
		const auto oldLength = m_length;
		NE_ASSERT(startIndex >= 0 && startIndex + length <= oldLength);

		if (startIndex == 0 && oldLength == length)
		{
			clear();
			return;
		}

		m_length = oldLength - length;
		m_data = (char*)neo::allocAligned((m_length + 1) * sizeof(char), 16);

		memcpy(m_data, oldData, startIndex * sizeof(char));
		memcpy(m_data + startIndex, oldData + startIndex + length, length * sizeof(char));
		m_data[m_length] = 0;

		neo::free(oldData);
	}


	void NString::split(char c, NArray<NString>& results) const
	{
		results.clear();
		i32 start = 0;
		i32 length = this->length();

		for (i32 i = 0; i < length; i++)
		{
			if (m_data[i] == c)
			{
				i32 count = i - start;
				if (count > 0)
				{
					results.add(substring(start, count));
				}
				start = i + 1;
			}
		}

		const i32 count = length - start;
		if (count > 0)
		{
			results.add(substring(start, count));
		}
	}
}