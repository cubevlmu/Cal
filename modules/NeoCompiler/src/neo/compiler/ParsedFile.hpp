/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/memory/Memory.hpp>

namespace neo
{
    class ASTNode;
 
    class NParsedFile
    {
    public:
        NParsedFile();
        ~NParsedFile() = default;

        void debugOutput(class NDebugOutput &);
        void saveTo(const char *path);
        bool loadFrom(const char *path);

        void clearNodes();

    public:
        Vector<ASTNode *> Nodes;
    };
}