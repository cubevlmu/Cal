#include "Logger.hpp"

#include <memory>
#include <sstream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "system/io/Path.hpp"

#define FORMAT "[%^%L%$] %v"

namespace cal {
    std::shared_ptr<Logger> LoggerManager::s_logger;

    void Logger::addLog(const char* val) { ss << val; }
    void Logger::addLog(cal::Path& val) { ss << val.c_str(); }
    void Logger::addLog(StringView val) { ss << val.toCString(); }
    void Logger::addLog(const std::string &val) { ss << val; }
    void Logger::addLog(u32 val) { ss << val; }
    void Logger::addLog(u16 val) { ss << val; }
    void Logger::addLog(u8 val) { ss << val; }
    void Logger::addLog(u64 val) { ss << val; }
    void Logger::addLog(char val) { ss << val; }
    void Logger::addLog(short val) { ss << val; }
    void Logger::addLog(long val) { ss << val; }
    void Logger::addLog(int val) { ss << val; }
    void Logger::addLog(float val) { ss << val; }
    void Logger::addLog(double val) { ss << val; }

    void Logger::init(const char* name) {
        std::string fileName(name);
        fileName.append(".log");

        std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true));

		logSinks[0]->set_pattern(FORMAT);
		logSinks[1]->set_pattern(FORMAT);

		m_backend = std::make_shared<spdlog::logger>(name, begin(logSinks), end(logSinks));
		spdlog::register_logger(m_backend);
		m_backend->set_level(spdlog::level::trace);
		m_backend->flush_on(spdlog::level::trace);
    }

    void Logger::emitLog(LogLevel level) {
        auto stdstr = ss.str();
        const char* str = stdstr.c_str();
        //TODO Add Log Callback Action To Here
        ss.str("");
        ss.clear();
        

        switch (level)
        {
        case LogWarn:
            m_backend->warn(str); break;
        case LogError:
            m_backend->error(str); break;
        case LogDebug:
            m_backend->debug(str); break;
        case LogInfo:
            m_backend->info(str); break;
        case LogTrace:
            m_backend->trace(str); break;
        }
    }
    
    std::shared_ptr<Logger> LoggerManager::getLogger() 
    { return s_logger; }
    
    void LoggerManager::createLogger() {
        s_logger = std::make_shared<Logger>();
    }

} // namespace cation::debug