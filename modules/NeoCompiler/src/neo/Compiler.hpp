// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include "neo/compiler/SourceDir.hpp"

#include <string>

namespace neo {

    class NCmdParser;

    struct CompilerConfig
    {
        std::string sourceDir;
    };


    class NCompiler 
    {
    public:
        NCompiler(int argc, char** argv);
        ~NCompiler();

        int runCompiler();

    private:
        static void regFlags(NCmdParser*);

    private:
        static CompilerConfig s_cfg;

        std::vector<NSourceDir> m_soruceDirs;
    };
}