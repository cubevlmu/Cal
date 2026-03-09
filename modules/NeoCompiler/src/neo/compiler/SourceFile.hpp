/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/memory/Memory.hpp>

namespace neo
{
    class NSourceFile final
    {
    public:
        NSourceFile(class NSourceDir *dir, String rPath);
        ~NSourceFile();

        bool readAll();
        StringView getContent() const;
        String getPath() const;
        String getFileName() const;
        String getLineText(psize line) const;

        bool compile();

    private:
        String m_rPath;
        String m_content;
        NSourceDir *m_dir;
    };

}
