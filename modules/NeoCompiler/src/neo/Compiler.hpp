/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/memory/Memory.hpp>

namespace neo {

    class NCmdParser;

    struct CompilerConfig final
    {
        String sourceDir;
    };


    class NCompiler final
    {
    public:
        NCompiler(int argc, char** argv);
        ~NCompiler();

        int runCompiler();

    private:
        static void regFlags(NCmdParser*);

    private:
        static CompilerConfig s_cfg;
    };
}
