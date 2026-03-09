/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/memory/Memory.hpp>
#include <neo/compiler/SourceFile.hpp>

namespace neo
{
    class NSourceDir
    {
    public:
        NSourceDir(const char *path);
        ~NSourceDir();

        bool collect();
        bool compile();

        StringView getRoot()
        {
            return m_path;
        }

    private:
        Vector<NSourceFile> m_sources;
        String m_path;
    };
}
