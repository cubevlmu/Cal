// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Compiler.hpp"

#include <nbase/utils/StringUtils.hpp>
#include <nbase/base/Logger.hpp>
#include <nbase/utils/CmdParser.hpp>
#include <nbase/utils/Timer.hpp>

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
        std::vector<std::string> out {};
        splitStr(out, s_cfg.sourceDir, ';');

        for (auto& str : out) {
            NSourceDir dir {str.c_str()};
            if (!dir.collect()) {
                LogDebug("No source file in dir {}", str);
            }
            m_soruceDirs.push_back(std::move(dir));
        }

        bool r = false;
        for (auto& dir : m_soruceDirs) {
            r |= dir.compile();
        }

        // generate process & link process

        t.end();
        if (!r) {
            LogError("Result occurrenced in compile process! Compiler halt in {} s.", t.secondTime());
			return 1;
        } else {
            LogInfo("Compiler process end in {} s.", t.secondTime());
			return 0;
        }
    }


}