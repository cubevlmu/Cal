// Created by cubevlmu on 2025/8/23.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <format>
#include <nbase/memory/Memory.hpp>

namespace neo {

	typedef neo::StlAllocator<char> fmt_allocator;
	typedef std::vector<char, neo::StlAllocator<char>> fmt_memory_buffer;

	NE_FORCE_INLINE static std::string_view toStdStringView(StringView text)
	{
		return std::string_view(text.data(), text.size());
	}

	/// Fmt-style string format function based on std::format. With custom memory buffer.
	template<typename... Args>
	NE_FORCE_INLINE static void format(fmt_memory_buffer& buffer, StringView format, const Args& ... args)
	{
		std::vformat_to(std::back_inserter(buffer), toStdStringView(format), std::make_format_args(args...));
	}

	/// Fmt-style string format function based on std::format. With custom memory buffer.
	template<typename... Args>
	NE_FORCE_INLINE static void format(fmt_memory_buffer& buffer, const char* format, const Args& ... args)
	{
		neo::format(buffer, StringView(format), args...);
	}

	/// Fmt-style string format function based on std::format
	template<typename... Args>
	static String format(StringView format, const Args& ... args)
	{
		neo::StlAllocator<char> allocator;
		neo::fmt_memory_buffer buf{allocator};
		neo::format(buf, format, args...);
		return String(buf.data(), static_cast<size_t>(buf.size()));
	}

	template<typename... Args>
	static String format(const char* format, const Args& ... args)
	{
		return neo::format(StringView(format), args...);
	}
}

#define DEFINE_DEFAULT_FORMATTING(type, formatText, ...)               \
    template<>                                                         \
    struct std::formatter<type, char> {                                \
        template<typename ParseContext>                                \
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }\
        template<typename FormatContext>                               \
        auto format(const type& v, FormatContext& ctx) const {         \
            return std::format_to(ctx.out(), formatText, __VA_ARGS__); \
        }                                                              \
    };

#define DEFINE_DEFAULT_FORMATTING_VIA_TO_STRING(type)                  \
    template<>                                                         \
    struct std::formatter<type, char> {                                \
        template<typename ParseContext>                                \
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }\
        template<typename FormatContext>                               \
        auto format(const type& v, FormatContext& ctx) const {         \
            auto str = v.toString();                                   \
            return std::ranges::copy(str.begin(), str.end(), ctx.out()).out; \
        }                                                              \
    };
