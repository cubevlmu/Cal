// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Logger.hpp"

#include <memory>
#include "nbase/utils/StringUtils.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#define FORMAT "[%^%L%$] %v"

namespace neo {

	static std::unique_ptr<spdlog::logger> s_logger;

	void Logger::emitLog(const std::string& msg, LogLevel level) {
		switch(level)
		{
		case LogLevel::kWarning:
			s_logger->warn(msg);
			break;
		case LogLevel::kError:
			s_logger->error(msg);
			break;
		case LogLevel::kDebug:
			s_logger->debug(msg);
			break;
		case LogLevel::kInfo:
			s_logger->info(msg);
			break;
		case LogLevel::kTrace:
			s_logger->trace(msg);
			break;
		}
	}

	Logger::Logger(const char *name, bool record)
	{
		auto fileName = concatStr(name, ".log");

		if (s_logger) {
			return;
		}

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks[0]->set_pattern(FORMAT);
		if (record) {
			logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true));
			logSinks[1]->set_pattern(FORMAT);
		}

		s_logger = std::make_unique<spdlog::logger>(name, begin(logSinks), end(logSinks));
		s_logger->set_level(spdlog::level::trace);
		s_logger->flush_on(spdlog::level::trace);
	}


	Logger* getNeoDefaultLogger(const char* name, bool useFileRecorder) {
		static Logger logger{name, useFileRecorder};
		return &logger;
	}

} // namespace neo