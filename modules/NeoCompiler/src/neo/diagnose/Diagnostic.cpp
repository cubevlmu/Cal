/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Diagnostic.hpp"

#include <algorithm>
#include <iostream>

namespace neo {

    namespace {
        const char* diagnosticLevelName(DiagnosticLevel level)
        {
            switch (level)
            {
            case DiagnosticLevel::kError:
                return "error";
            case DiagnosticLevel::kWarning:
                return "warning";
            case DiagnosticLevel::kNote:
                return "note";
            case DiagnosticLevel::kHint:
                return "hint";
            default:
                return "diagnostic";
            }
        }
    }

    void DiagnosticCollector::report(DiagnosticLevel level, const SourceLoc& loc, const String& message)
    {
        m_diagnostics.emplace_back(Diagnostic{ level, loc, message });
        if (level == DiagnosticLevel::kError)
            ++m_errorCount;
    }


    void DiagnosticCollector::printAll() const
    {
        for (auto& dig : m_diagnostics)
        {
            printOne(dig);
        }
    }

    void DiagnosticCollector::printOne(const Diagnostic& diagnostic) const
    {
        std::cerr << diagnostic.location.toString()
                  << ": "
                  << diagnosticLevelName(diagnostic.level)
                  << ": "
                  << diagnostic.message
                  << '\n';

        if (diagnostic.location.file == nullptr || diagnostic.location.line == 0) {
            return;
        }

        const String sourceLine = diagnostic.location.file->getLineText(diagnostic.location.line);
        if (sourceLine.empty()) {
            return;
        }

        std::cerr << "  |\n";
        std::cerr << diagnostic.location.line << " | " << sourceLine << '\n';
        std::cerr << "  | ";

        const psize caretColumn = diagnostic.location.column > 0 ? diagnostic.location.column : 1;
        for (psize i = 1; i < caretColumn; ++i) {
            const char ch = i - 1 < sourceLine.size() ? sourceLine[i - 1] : ' ';
            std::cerr << (ch == '\t' ? '\t' : ' ');
        }
        std::cerr << "^\n";
    }


    void DiagnosticCollector::clear(DiagnosticLevel flags)
    {
        if (flags == DiagnosticLevel::kNone) {
            m_diagnostics.clear();
            return;
        }
        m_diagnostics.erase(std::remove_if(m_diagnostics.begin(), m_diagnostics.end(), [&flags](auto& item) -> bool {
            return item.level == flags;
        }), m_diagnostics.end());
    }


}
