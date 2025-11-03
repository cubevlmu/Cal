// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <nbase/base/Assert.hpp>

namespace neo {

	/// Universal representation of a contiguous region of arbitrary memory
	template<typename T>
	class NSpan
	{
	public:
		NSpan()
			: m_data(nullptr)
			, m_length(0) {
		}
		NSpan(const T* data, i32 length)
			: m_data((T*)data)
			, m_length(length) {
		}
		NSpan(T* begin, T* end)
			: m_data(begin)
			, m_length(end - begin) {
		}
		template <int N>
		NSpan(T(&value)[N])
			: m_data(value)
			, m_length(N) {
		}

		NE_FORCE_INLINE bool isValid() const
		{
			return m_data != nullptr;
		}
		NE_FORCE_INLINE bool isInvalid() const
		{
			return m_data == nullptr;
		}

		NE_FORCE_INLINE i32 length() const
		{
			return m_length;
		}

		NE_FORCE_INLINE T* get()
		{
			return m_data;
		}
		NE_FORCE_INLINE const T* get() const
		{
			return m_data;
		}
		template<typename U>
		NE_FORCE_INLINE U* get() const
		{
			return (U*)m_data;
		}

		NE_FORCE_INLINE T& operator[](i32 index)
		{
			NE_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}
		NE_FORCE_INLINE const T& operator[](i32 index) const
		{
			NE_ASSERT(index >= 0 && index < m_length);
			return m_data[index];
		}

		NE_FORCE_INLINE T* begin()
		{
			return m_data;
		}

		NE_FORCE_INLINE T* end()
		{
			return m_data + m_length;
		}

		NE_FORCE_INLINE const T* begin() const
		{
			return m_data;
		}

		NE_FORCE_INLINE const T* end() const
		{
			return m_data + m_length;
		}

	protected:
		T* m_data;
		i32 m_length;
	};

	template<typename T>
	inline NSpan<T> toNSpan(const T* ptr, i32 length)
	{
		return NSpan<T>(ptr, length);
	}

	template<typename T>
	inline bool spanContains(const NSpan<T> span, const T& value)
	{
		for (i32 i = 0; i < span.length(); i++)
		{
			if (span.get()[i] == value)
				return true;
		}
		return false;
	}
}