// Created by cubevlmu on 2025/11/24.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/memory/Memory.hpp>

namespace neo {

	template <typename T>
	struct ScopeGuard
	{
		ScopeGuard(T* ptr) : m_ptr{ ptr }, m_used{ false } {}
		~ScopeGuard() {
			if (m_used)
				return;
			neo::deletePtr(m_ptr);
		}

		T* getPtr() {
			m_used = true;
			return m_ptr;
		}

		T* operator->() {
			return m_ptr;
		}

	private:
		bool m_used;
		T* m_ptr;
	};

}