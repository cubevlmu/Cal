// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "DebugOutput.hpp"

#include <filesystem>

namespace neo {

    NFileOutput::NFileOutput(const std::string_view& path)
    {
        if (std::filesystem::exists(path.data())) {
            std::filesystem::remove(path.data());
        }

        m_fs.open(path.data());
        if (!m_fs.is_open()) {
            LogError("Failed to open file");
        }
    }


    NFileOutput::~NFileOutput()
    {
        m_fs.close();
    }

    void NFileOutput::writeLine(const std::string_view& line) {
        m_fs << line.data() << '\n';
    }

    void NFileOutput::write(const std::string_view& line) {
        m_fs << line.data();
    }

    bool NFileOutput::print() {
        // printm_fs.flags();
        return true;
    }
}