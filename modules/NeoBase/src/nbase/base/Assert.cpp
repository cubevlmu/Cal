// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Assert.hpp"
#include <nbase/common.hpp>

#if NE_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

#if NE_WINDOWS
#define NE_DEBUG_BREAK() __debugbreak()
#else
#define NE_DEBUG_BREAK()  raise(SIGTRAP)
#endif

namespace neo {

#if NE_WINDOWS
	template<typename Lambda>
	inline void DumpStack(Lambda&& lambda)
	{
		const int maxFrames = 64;
		void* frames[maxFrames] = { 0 };

		USHORT captured = RtlCaptureStackBackTrace(0, maxFrames, frames, nullptr);
		if (captured == 0) return;

		HANDLE process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);

		for (USHORT i = 0; i < captured; ++i)
		{
			DWORD64 address = (DWORD64)frames[i];

			char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = { 0 };
			PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
			pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			pSymbol->MaxNameLen = MAX_SYM_NAME;

			DWORD64 displacement = 0;
			if (SymFromAddr(process, address, &displacement, pSymbol))
			{
				lambda(pSymbol->Name);
			}
			else
			{
				lambda("Unknown");
			}
		}
	}

	void getStackTrace(const stack_trace_func f) {
		DumpStack(f);
	}
#endif

	void assertIt(const char* msg, const char* func, int line, const char* file)
	{
#if NE_WINDOWS
		char txt[512];
		memset(&txt[0], 0, sizeof(char) * 512);
		snprintf((char*)&txt[0], 512, "%s\n\n%s:%d %s", msg, file, line, func);
		if(!IsDebuggerPresent())
		{
			MessageBoxA(nullptr, &txt[0], "Assert failed", MB_OK | MB_ICONERROR);
		} else {
			fprintf(stderr, "Assert failed %s", (const char*)&txt[0]);
		}
#elif NE_MACOS
		#elif NE_ANDROID
#elif NE_LINUX
#else
        fprintf(stderr, "Assertion failed: %s (%s:%d %s)\n", msg, file, line, func);
#endif
		printf("| StackTrace:\n");
		getStackTrace([](const char* tr) {
			printf("|> %s\n", tr);
		});
		NE_DEBUG_BREAK();
	}

}