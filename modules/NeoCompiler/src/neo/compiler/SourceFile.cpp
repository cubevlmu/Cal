/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "SourceFile.hpp"

#include "neo/compiler/SourceDir.hpp"
#include "neo/compiler/Lexer.hpp"
#include "neo/compiler/Parser.hpp"
#include "neo/compiler/ParsedFile.hpp"
#include "neo/analyzer/SemanticAnalyzer.hpp"
#include "DebugOutput.hpp"
#include "nbase/utils/Timer.hpp"

#include <nbase/base/Format.hpp>
#include <nbase/base/Logger.hpp>
#include <nbase/utils/StringUtils.hpp>

#include <fstream>
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

namespace neo
{

    NSourceFile::NSourceFile(NSourceDir *s, String rp)
        : m_rPath{std::move(rp)}, m_content{}, m_dir{s}
    {
    }

    NSourceFile::~NSourceFile()
    {
        m_content.clear();
    }

    bool NSourceFile::readAll()
    {
        std::ifstream stm{};
        stm.open(getPath().c_str());
        if (!stm.is_open())
        {
            std::cerr << getPath() << ":1:1: error: failed to read source file\n";
            return false;
        }

        String buf{};
        OStringStream builder{};
        while (std::getline(stm, buf))
        {
            builder << buf << '\n';
        }
        buf.clear();
        m_content = builder.str();

        return true;
    }

    StringView NSourceFile::getContent() const
    {
        if (m_content.empty())
        {
            return "";
        }
        return m_content;
    }

    String NSourceFile::getFileName() const
    {
        fs::path p{m_rPath.c_str()};
        return String(p.filename().string().c_str());
    }

    String NSourceFile::getLineText(psize line) const
    {
        if (line == 0 || m_content.empty())
        {
            return {};
        }

        psize currentLine = 1;
        psize start = 0;

        while (start < m_content.size() && currentLine < line)
        {
            if (m_content[start] == '\n')
            {
                ++currentLine;
            }
            ++start;
        }

        if (currentLine != line || start >= m_content.size())
        {
            return {};
        }

        psize end = start;
        while (end < m_content.size() && m_content[end] != '\n' && m_content[end] != '\r')
        {
            ++end;
        }

        return m_content.substr(start, end - start);
    }

    String NSourceFile::getPath() const
    {
        std::filesystem::path p(m_dir->getRoot().data());
        p /= m_rPath.c_str();
        return p.lexically_normal().string().c_str();
    }

    bool NSourceFile::compile()
    {
        if (!readAll())
        {
            return false;
        }

        NTimer tm_lexer{};
        NLexer lex{this};
        if (!lex.lex())
        {
            lex.printDiagnostics();
            return false;
        }
        LogDebug("Lexer lex file in {}ms", tm_lexer.milliTime());

        NTimer tm_lex{};
        NFileOutput o{neo::format("output_lex_{}.txt", m_rPath)};
        lex.debugPrint(o);
        o.print();
        LogDebug("Build Lexer's output in {}ms", tm_lex.milliTime());

        NTimer tm_parser{};
        NParsedFile file{};
        NParserArgs args{
            .lexer = &lex,
            .file = this,
            .output = file,
            .langVer = 1,
        };
        NParser parser{args};
#if NE_DEBUG
        if (!parser.debugParse())
        {
            return false;
        }
        LogDebug("Parser parse in {}ms", tm_parser.milliTime());

        NTimer tm_sema{};
        DiagnosticCollector semaDiag{};
        SemanticAnalyzer sema{semaDiag};
        if (auto r = sema.analyze(file); r.hasError())
        {
            semaDiag.printAll();
            return false;
        }
        LogDebug("Semantic analyze in {}ms", tm_sema.milliTime());

        NTimer tm{};
        NFileOutput op{neo::format("output_ast_{}.txt", m_rPath)};
        op.beginRoot("ParserResult");
        op.printItem("source", m_rPath);
        op.printChildren("nodes", file.Nodes);
        op.endRoot();
        op.print();
        LogDebug("Build AST Output in {}ms", tm.milliTime());
#else
        if (!parser.parse())
        {
            return false;
        }

        DiagnosticCollector semaDiag{};
        SemanticAnalyzer sema{semaDiag};
        if (auto r = sema.analyze(file); r.hasError())
        {
            semaDiag.printAll();
            return false;
        }
#endif

        return true;
    }

}
