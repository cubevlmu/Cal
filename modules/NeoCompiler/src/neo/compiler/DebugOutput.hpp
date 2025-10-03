// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <fstream>
#include <sstream>

#include <nbase/base/Logger.hpp>

namespace neo {

    class NDebugOutput
    {
    public:
        virtual ~NDebugOutput() = default;

        virtual void writeLine(const std::string_view& line) = 0;
        virtual void write(const std::string_view& txt) = 0;

        template <typename... Args>
        NE_FORCE_INLINE void write(std::string_view fmtStr, const Args&... args) {
              write(neo::format(fmtStr, args...));
        }
        template <typename... Args>
        NE_FORCE_INLINE void writeLine(std::string_view fmtStr, const Args&... args) {
            writeLine(neo::format(fmtStr, args...));
        }

        virtual bool print() = 0;
    };


    class NConsoleOutput final : public NDebugOutput
    {
    public:
        ~NConsoleOutput() override = default;

        void writeLine(const std::string_view& line) override {
            m_ss << line.data();
            LogInfo("{}", m_ss.str());

            m_ss.clear();
            m_ss.str("");
        }
        void write(const std::string_view& txt) override {
            m_ss << txt.data();
        }
        bool print() override { return true; }

    private:
        std::stringstream m_ss;
    };


    class NFileOutput final : public NDebugOutput
    {
    public:
        explicit NFileOutput(const std::string_view& path);
        ~NFileOutput() override;

        void writeLine(const std::string_view& line) override;
        void write(const std::string_view& txt) override;
        bool print() override;

    private:
        std::ofstream m_fs;
    };
    
}
