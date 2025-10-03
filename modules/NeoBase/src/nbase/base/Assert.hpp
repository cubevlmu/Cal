// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

namespace neo {

	/// Processing function defination of back trace
	typedef void(*stack_trace_func)(const char*);

	/// Get the hole stack trace for debug. Processing by trace function
	void getStackTrace(const stack_trace_func);

	/// Assert body
	void assertIt(const char* msg, const char* func, int line, const char* file);

}

#ifdef NE_DEBUG
#define NE_ASSERTATION 1
#define NE_ASSERT(COND) do { if (!(COND)) { ::neo::assertIt(#COND, __func__, __LINE__, __FILE__); } } while(false)
#else
#define NE_ASSERTATION 0
#define NE_ASSERT(COND) ((void)(COND))
#endif