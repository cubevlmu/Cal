// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "SourceFile.hpp"

#include "neo/compiler/SourceDir.hpp"
#include "neo/compiler/Lexer.hpp"
#include "neo/compiler/Parser.hpp"
#include "neo/compiler/ParsedFile.hpp"
#include "DebugOutput.hpp"

#include <nbase/base/Logger.hpp>
#include <nbase/utils/StringUtils.hpp>

#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

namespace neo {

    NSourceFile::NSourceFile(NSourceDir* s, std::string rp)
        : m_rPath {std::move(rp)}
        , m_content {}
        , m_dir {s}
    {
    }

    NSourceFile::~NSourceFile()
    {
        m_content.clear();
    }

    bool NSourceFile::readAll()
    {
        std::ifstream stm {};
        stm.open(getPath());
        if (!stm.is_open()) {
            LogError("Failed to read soruce file {}", m_rPath);
            return false;
        }
        
        std::string buf {};
        std::stringstream builder {};
        while (std::getline(stm, buf)) {
            builder << buf << '\n';
        }
        buf.clear();
        m_content = builder.str();

        return true;
    }

    std::string_view NSourceFile::getContent() const
    {
        if (m_content.empty()) {
            return "";
        }
        return m_content;
    }

    std::string NSourceFile::getFileName() const
    {
        fs::path p {m_rPath};
        return p.filename().string();
    }

    std::string NSourceFile::getPath() const {
        return concatStr(m_dir->getRoot().data(), "//", m_rPath.c_str());
    }

    bool NSourceFile::compile() {
        if (!readAll()) {
            return false;
        }

        NLexer lex {this};
        if (!lex.lex()) {
            LogError("Failed to lex file lex.neo");
            return -1;
        }

        NFileOutput o {"output_lex.txt"};
        lex.debugPrint(o);

        NParsedFile file {};
        NParserArgs args {
            .lexer = &lex,
            .file = this,
            .output = file,
            .langVer = 1,
        };
        NParser parser {args};
#if NE_DEBUG
        if (!parser.debugParse()) {
            return false;
        }
        NConsoleOutput op {};
        for (const auto &item: file.Nodes) {
            item->debugPrint(op);
            op.writeLine("");
        }
#else
        if (!parser.parse()) {
            return false;
        }
#endif

        // Symbol collect pass & analyzer pass

        return true;
    }

}
