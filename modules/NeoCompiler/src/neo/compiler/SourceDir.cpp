// Created by cubevlmu on 2025/8/16.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "SourceDir.hpp"

#include <nbase/utils/StringUtils.hpp>
#include <nbase/base/Logger.hpp>

#include <filesystem>
namespace fs = std::filesystem;

namespace neo {

    NSourceDir::NSourceDir(const char *path)
        : m_path {path}
    {
    }

    NSourceDir::~NSourceDir() {

    }

    bool NSourceDir::collect() {
        if (!std::filesystem::exists(m_path)) {
            return false;
        }

        for (const auto& entry : fs::recursive_directory_iterator(m_path)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".neo")
                continue;
            auto pth = fs::relative(entry, m_path).string();
            LogDebug("Neo source file : {} / {}", m_path, pth);
            m_sources.insert({pth, NSourceFile {this, std::move(pth)}});
        }

        return true;
    }

    bool NSourceDir::compile() {
        bool r = false;

        for (auto& [_,f] : m_sources) {
            r |= f.compile();
        }

        return r;
    }
}