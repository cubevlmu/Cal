#pragma once


#include <stdint.h>
#include <string>
#include <sstream>
#include <memory>
#include "globals.hpp"

namespace spdlog {
    class logger;
}

namespace concept {

    enum LogLevel {
        LogWarn, LogError, LogDebug, LogInfo, LogTrace
    };

    class Logger
    {
    private:
        void addLog(const char* val);
        void addLog(const std::string& val);
        void addLog(u64 val);
        void addLog(u32 val);

        void addLog(u16 val);
        void addLog(u8 val);

        void addLog(char val);
        void addLog(short val);
        void addLog(long val);
        void addLog(int val);

        void addLog(float val);
        void addLog(double val);

        void emitLog(LogLevel level);

        template <typename... T> 
        void log(LogLevel level, const T&... args) {
            int tmp[] = { (addLog(args), 0) ... };
            (void)tmp;
            emitLog(level);
        }

    public:
        Logger(const char* name = "concept") {
            init(name);
        }

        void init(const char* name = "concept");

        template <typename... T> 
        void info(const T&... args) 
        { log(LogLevel::LogInfo, args...); }

        template <typename... T> 
        void warn(const T&... args) 
        { log(LogLevel::LogWarn, args...); }

        template <typename... T> 
        void error(const T&... args) 
        { log(LogLevel::LogError, args...); }

        template <typename... T> 
        void debug(const T&... args) 
        { log(LogLevel::LogDebug, args...); }

        template <typename... T> 
        void trace(const T&... args) 
        { log(LogLevel::LogTrace, args...); }

    private:
        std::stringstream ss;
        std::shared_ptr<spdlog::logger> m_backend;
    };

    class LoggerManager {
    public:
        static std::shared_ptr<Logger> getLogger();
        static void createLogger();
    private:
        static std::shared_ptr<Logger> s_logger;
    };
}

#define InitLogger() ::concept::LoggerManager::createLogger()

#define LogInfo(...)  ::concept::LoggerManager::getLogger()->info(__VA_ARGS__)
#define LogWarn(...)  ::concept::LoggerManager::getLogger()->warn(__VA_ARGS__)
#define LogError(...) ::concept::LoggerManager::getLogger()->error(__VA_ARGS__)
#define LogDebug(...) ::concept::LoggerManager::getLogger()->debug(__VA_ARGS__)
#define LogTrace(...) ::concept::LoggerManager::getLogger()->trace(__VA_ARGS__)