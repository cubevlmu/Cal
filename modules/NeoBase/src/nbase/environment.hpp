#pragma once

#define _NE_STR_HELPER(x) #x
#define _NE_STR(x) _NE_STR_HELPER(x)

#define NE_SAFE_ALIGNMENT 16
#define NE_CACHE_LINE_SIZE 64

#if defined(__clang__)
#	define NE_COMPILER_CLANG 1
#	define NE_COMPILER_GCC 0
#	define NE_COMPILER_MSVC 0
#	define NE_COMPILER_GCC_COMPATIBLE 1
#	define NE_COMPILER_STR "clang " __VERSION__
#elif defined(__GNUC__) || defined(__GNUG__)
#	define NE_COMPILER_CLANG 0
#	define NE_COMPILER_GCC 1
#	define NE_COMPILER_MSVC 0
#	define NE_COMPILER_GCC_COMPATIBLE 1
#	define NE_COMPILER_STR "gcc " __VERSION__
#elif defined(_MSC_VER)
#	define NE_COMPILER_CLANG 0
#	define NE_COMPILER_GCC 0
#	define NE_COMPILER_MSVC 1
#	define NE_COMPILER_GCC_COMPATIBLE 0
#	define NE_COMPILER_STR "MSVC " _NE_STR(_MSC_FULL_VER)
#else
#	error Unknown compiler
#endif

// Operating system
#if defined(__WIN32__) || defined(_WIN32)
#	define NE_LINUX 0
#	define NE_ANDROID 0
#	define NE_MACOS 0
#	define NE_IOS 0
#	define NE_WINDOWS 1
#	define NE_STR "Windows"
#elif defined(__APPLE_CC__)
#	if (defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 40000) || (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 40000)
#		define NE_LINUX 0
#		define NE_ANDROID 0
#		define NE_MACOS 0
#		define NE_IOS 1
#		define NE_WINDOWS 0
#		define NE_STR "iOS"
#	else
#		define NE_LINUX 0
#		define NE_ANDROID 0
#		define NE_MACOS 1
#		define NE_IOS 0
#		define NE_WINDOWS 0
#		define NE_STR "MacOS"
#	endif
#elif defined(__ANDROID__)
#	define NE_LINUX 0
#	define NE_ANDROID 1
#	define NE_MACOS 0
#	define NE_IOS 0
#	define NE_WINDOWS 0
#	define NE_STR "Android"
#elif defined(__linux__)
#	define NE_LINUX 1
#	define NE_ANDROID 0
#	define NE_MACOS 0
#	define NE_IOS 0
#	define NE_WINDOWS 0
#	define NE_STR "Linux"
#else
#	error Unknown OS
#endif


#if NE_LINUX || NE_ANDROID || NE_MACOS || NE_IOS
#	define NE_POSIX 1
#else
#	define NE_POSIX 0
#endif


#if NE_COMPILER_GCC_COMPATIBLE
#	if defined(__arm__) || defined(__aarch64__)
#		define NE_CPU_ARCH_X86 0
#		define NE_CPU_ARCH_ARM 1
#	elif defined(__i386__) || defined(__amd64__)
#		define NE_CPU_ARCH_X86 1
#		define NE_CPU_ARCH_ARM 0
#	endif
#elif NE_COMPILER_MSVC
#	if defined(_M_ARM)
#		define NE_CPU_ARCH_X86 0
#		define NE_CPU_ARCH_ARM 1
#	elif defined(_M_X64) || defined(_M_IX86)
#		define NE_CPU_ARCH_X86 1
#		define NE_CPU_ARCH_ARM 0
#	endif
#endif


#if defined(NE_CPU_ARCH_X86) && NE_CPU_ARCH_X86
#	define NE_GPU_ARCH_STR "x86"
#elif defined(NE_CPU_ARCH_ARM) && NE_CPU_ARCH_ARM
#	define NE_GPU_ARCH_STR "arm"
#else
#	error Unknown CPU arch
#endif


#if !NE_ENABLE_SIMD
#	define NE_SIMD_NONE 1
#	define NE_SIMD_SSE 0
#	define NE_SIMD_NEON 0
#	define NE_SIMD_STR "None"
#elif NE_CPU_ARCH_X86
#	define NE_SIMD_NONE 0
#	define NE_SIMD_SSE 1
#	define NE_SIMD_NEON 0
#	define NE_SIMD_STR "SSE"
#else
#	define NE_SIMD_NONE 0
#	define NE_SIMD_SSE 0
#	define NE_SIMD_NEON 1
#	define NE_SIMD_STR "Neon"
#endif


#if NE_COMPILER_GCC_COMPATIBLE
#	define NE_RESTRICT __restrict
#	define NE_FORCE_INLINE inline __attribute__((always_inline))
#	define NE_DONT_INLINE __attribute__((noinline))
#	define NE_UNUSED __attribute__((__unused__))
#	define NE_COLD __attribute__((cold, optimize("Os")))
#	define NE_HOT __attribute__ ((hot))
#	define NE_UNREACHABLE() __builtin_unreachable()
#	define NE_PREFETCH_MEMORY(addr) __builtin_prefetch(addr)
#	define NE_CHECK_FORMAT(fmtArgIdx, firstArgIdx) __attribute__((format(printf, fmtArgIdx + 1, firstArgIdx + 1))) // On methods you need to include "this"
#	define NE_PURE __attribute__((pure))
#elif NE_COMPILER_MSVC
#	define NE_RESTRICT
#	define NE_FORCE_INLINE __forceinline
#	define NE_DONT_INLINE
#	define NE_UNUSED
#	define NE_COLD
#	define NE_HOT
#	define NE_UNREACHABLE() __assume(false)
#	define NE_PREFETCH_MEMORY(addr) (void)(addr)
#	define NE_CHECK_FORMAT(fmtArgIdx, firstArgIdx)
#	define NE_PURE
#else
#	define NE_RESTRICT
#	define NE_FORCE_INLINE
#	define NE_DONT_INLINE
#	define NE_UNUSED
#	define NE_COLD
#	define NE_HOT
#	define NE_UNREACHABLE()
#	define NE_PREFETCH_MEMORY(addr) (void)(addr)
#	define NE_CHECK_FORMAT(fmtArgIdx, firstArgIdx)
#	define NE_PURE
#endif

#include <nbase/definitions.hpp>