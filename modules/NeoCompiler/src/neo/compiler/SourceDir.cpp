/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "SourceDir.hpp"

#include <nbase/utils/StringUtils.hpp>
#include <nbase/base/Logger.hpp>

#include <filesystem>
namespace fs = std::filesystem;

namespace neo
{
    NSourceDir::NSourceDir(const char *path)
        : m_path{path}
    {
    }

    NSourceDir::~NSourceDir()
    {
    }

    bool NSourceDir::collect()
    {
        if (!std::filesystem::exists(m_path))
        {
            return false;
        }

        for (const auto &entry : fs::recursive_directory_iterator(m_path))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".neo")
                continue;
            auto pth = String{fs::relative(entry, m_path.c_str()).string().c_str()};

            std::filesystem::path p(m_path.data());
            p /= pth.c_str();
            LogDebug("Neo source file : {}", p.lexically_normal().string());
            m_sources.push_back(NSourceFile{this, std::move(pth)});
        }

        return true;
    }

    bool NSourceDir::compile()
    {
        bool hasSource = false;
        bool r = true;

        for (auto &f : m_sources)
        {
            hasSource = true;
            r &= f.compile();
        }

        return hasSource && r;
    }
}
