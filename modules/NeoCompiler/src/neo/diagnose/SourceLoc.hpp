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
    struct SourceLoc final
    {
        psize line;
        psize column;
        class NSourceFile *file;

    public:
        SourceLoc() : line{0}, column{0}, file{nullptr} {}
        SourceLoc(psize line, psize column, class NSourceFile *file)
            : line{line}, column{column}, file{file} {}

    public:
        String path() const;
        String toString() const;
        void write(class NSerializer *) const;
        void read(class NSerializer *);
    };

}
