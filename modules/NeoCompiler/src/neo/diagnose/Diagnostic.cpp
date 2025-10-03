// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Diagnostic.hpp"

#include <nbase/base/Logger.hpp>

namespace neo {

    void DiagnosticCollector::report(DiagnosticLevel level, const SourceLoc& loc, const std::string& message)
    {
        m_diagnostics.emplace_back(Diagnostic{ level, loc, message });
        if (level == DiagnosticLevel::kError)
            ++m_errorCount;
    }


    void DiagnosticCollector::printAll() const
    {
        for (auto& dig : m_diagnostics)
        {
            switch (dig.level)
            {
            case DiagnosticLevel::kError:
                LogError("{} | {}", dig.location.toString(), dig.message);
                break;
            case DiagnosticLevel::kWarning:
                LogWarn("{} | {}", dig.location.toString(), dig.message);
                break;
            case DiagnosticLevel::kNote:
                LogInfo("{} | {}", dig.location.toString(), dig.message);
                break;
            default:
                break;
            }
        }
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