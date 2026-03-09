/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ParsedFile.hpp"

#include "nbase/memory/Memory.hpp"
#include "neo/ast/Base.hpp"

namespace neo
{

    NParsedFile::NParsedFile()
    {
    }

    void NParsedFile::clearNodes()
    {
        for (auto *item : Nodes)
        {
            neo::deletePtr(item);
        }
        Nodes.clear();
    }

} // namespace neo
