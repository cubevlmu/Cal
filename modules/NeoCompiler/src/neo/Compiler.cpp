/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Compiler.hpp"
#include "neo/compiler/SourceDir.hpp"

#include <nbase/utils/StringUtils.hpp>
#include <nbase/base/Logger.hpp>
#include <nbase/utils/CmdParser.hpp>
#include <nbase/utils/Timer.hpp>

#include <nbase/types/String.hpp>
#include <nbase/types/Array.hpp>

#include <iostream>

namespace neo {

    CompilerConfig NCompiler::s_cfg{
        .sourceDir = {}
    };

    NCompiler::NCompiler(int argc, char **argv) {
        NCmdParser cmdPs{argc, argv};
        NCompiler::regFlags(&cmdPs);
        if (!cmdPs.parse()) {
            std::cerr << "Invalid arguments! Failed to parse arguments\n";
            std::exit(-1);
        }
    }

    NCompiler::~NCompiler() {
    }

    void NCompiler::regFlags(neo::NCmdParser* p) {
        p->regStr("srcDir", s_cfg.sourceDir);
    }

    int NCompiler::runCompiler() {
        if (s_cfg.sourceDir.empty()) {
            LogError("No source dir input! Compiler halt.");
            return 1;
        }

        NTimer t{};

        NString path {s_cfg.sourceDir.c_str()};
		NArray<NString> results{};
		path.split(';', results);

        bool hasInput = false;
        bool r = true;
        for (auto& str : results) {
            NSourceDir dir {str.get()};
            if (!dir.collect()) {
                LogDebug("No source file in dir {}", str);
                continue;
            }
            hasInput = true;
            r &= dir.compile();
        }

        // generate process & link process

        if (!hasInput || !r) {
            LogError("Compilation failed in {} ms.", t.milliTime());
			return 1;
        } else {
            LogInfo("Compiler process end in {} ms.", t.milliTime());
			return 0;
        }
    }


}
