// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <nbase/base/Format.hpp>

#include <format>

namespace neo {

	enum class LogLevel : u8 {
		kWarning,
		kError,
		kDebug,
		kInfo,
		kTrace
	};

	class Logger final {
	private:
		void emitLog(const std::string& msg, LogLevel level);

	public:
		Logger(const char* name, bool record);
		~Logger() = default;

		template <typename... Args>
		NE_FORCE_INLINE void info(std::string_view fmtStr, const Args&... args) {
			emitLog(neo::format(fmtStr, std::forward<const Args&>(args)...),  LogLevel::kInfo);
		}
		NE_FORCE_INLINE void info(const char* msg) {
			emitLog(msg, LogLevel::kInfo);
		}

		template <typename... Args>
		NE_FORCE_INLINE void warning(std::string_view fmtStr, const Args&... args) {
			emitLog(neo::format(fmtStr, std::forward<const Args&>(args)...), LogLevel::kWarning);
		}
		NE_FORCE_INLINE void warning(const char* msg) {
			emitLog(msg, LogLevel::kWarning);
		}

		template <typename... Args>
		NE_FORCE_INLINE void error(std::string_view fmtStr, Args&&... args) {
			emitLog(neo::format(fmtStr, std::forward<const Args&>(args)...),  LogLevel::kError);
		}
		NE_FORCE_INLINE void error(const char* msg) {
			emitLog(msg, LogLevel::kError);
		}

		template <typename... Args>
		NE_FORCE_INLINE void debug(std::string_view fmtStr, const Args&... args) {
#if NE_DEBUG
			emitLog(neo::format(fmtStr, std::forward<const Args&>(args)...),  LogLevel::kDebug);
#else
			(void)fmtStr;
            ((void)args, ...);
#endif
		}
		NE_FORCE_INLINE void debug(const char* msg) {
#if NE_DEBUG
			emitLog(msg, LogLevel::kDebug);
#else
			(void)msg;
#endif
		}

		template <typename... Args>
		NE_FORCE_INLINE void trace(std::string_view fmtStr, const Args&... args) {
#if NE_DEBUG
			emitLog(neo::format(fmtStr, std::forward<const Args&>(args)...),  LogLevel::kTrace);
#else
			(void)fmtStr;
            ((void)args, ...);
#endif
		}
		NE_FORCE_INLINE void trace(const char* msg) {
#if NE_DEBUG
			emitLog(msg, LogLevel::kTrace);
#else
			(void)msg;
#endif
		}
	};

	Logger* getNeoDefaultLogger(const char* name = "neo", bool useFileRecorder = false);
}

#define LogInfo(STR, ...)  ::neo::getNeoDefaultLogger()->info(STR, __VA_ARGS__)
#define LogWarn(STR, ...)  ::neo::getNeoDefaultLogger()->warning(STR, __VA_ARGS__)
#define LogError(STR, ...) ::neo::getNeoDefaultLogger()->error(STR, __VA_ARGS__)
#define LogDebug(STR, ...) ::neo::getNeoDefaultLogger()->debug(STR, __VA_ARGS__)
#define LogTrace(STR, ...) ::neo::getNeoDefaultLogger()->trace(STR, __VA_ARGS__)