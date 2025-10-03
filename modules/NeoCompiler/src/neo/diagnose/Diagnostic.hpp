// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include "SourceLoc.hpp"
#include "neo/compiler/SourceFile.hpp"
#include "neo/compiler/Tokens.hpp"

#include <nbase/base/Assert.hpp>
#include <vector>
#include <type_traits>

namespace neo {

    enum class DiagnosticLevel : u32 {
        kNone = 0,
        kError = 1 << 0,
        kWarning = 1 << 1,
        kNote = 1 << 2,
    };


    struct Diagnostic {
        DiagnosticLevel level;
        SourceLoc location;
        std::string message;
    };


    class DiagnosticCollector
    {
    public:
        DiagnosticCollector() = default;

    public:
        void report(DiagnosticLevel level, const SourceLoc& loc, const std::string& message);

        NE_FORCE_INLINE void error(const SourceLoc& loc, const std::string& msg) {
            report(DiagnosticLevel::kError, loc, msg);
        }
        NE_FORCE_INLINE void warning(const SourceLoc& loc, const std::string& msg) {
            report(DiagnosticLevel::kWarning, loc, msg);
        }
        NE_FORCE_INLINE void note(const SourceLoc& loc, const std::string& msg) {
            report(DiagnosticLevel::kNote, loc, msg);
        }

        NE_FORCE_INLINE bool hasError() const { return m_errorCount > 0; }
        NE_FORCE_INLINE int getErrorCount() const { return m_errorCount; }
        const std::vector<Diagnostic>& diagnostics() const { return m_diagnostics; }

        void printAll() const;
        void clear(DiagnosticLevel flags = DiagnosticLevel::kNone);

    private:
        std::vector<Diagnostic> m_diagnostics;
        int m_errorCount = 0;
    };


    template<typename T>
    class ErrorOr {
    public:
        static ErrorOr<T> success(T value) {
            return ErrorOr(std::move(value));
        }

        static ErrorOr<T> failure(std::string errorMsg) {
            return ErrorOr(std::move(errorMsg));
        }

        bool hasError() const { return m_hasError; }
        const std::string& errorMessage() const { return m_errorMessage; }
        const T& value() const { NE_ASSERT(!m_hasError); return m_value; }
        T& value() { NE_ASSERT(!m_hasError); return m_value; }

        explicit operator bool() const { return !m_hasError; }

    private:
        ErrorOr(T val)
            : m_value(std::move(val)), m_hasError(false) {
        }

        ErrorOr(std::string msg)
            : m_errorMessage(std::move(msg)), m_hasError(true) {
        }

        T m_value;
        std::string m_errorMessage;
        bool m_hasError = true;
    };


    template<>
    class ErrorOr<void> {
    public:
        static ErrorOr<void> success() {
            return {};
        }

        static ErrorOr<void> failure(std::string errorMsg) {
            return {std::move(errorMsg)};
        }

        bool hasError() const { return m_hasError; }
        const std::string& errorMessage() const { return m_errorMessage; }

        explicit operator bool() const { return !m_hasError; }

    private:
        ErrorOr() : m_hasError(false) {}
        ErrorOr(std::string msg)
            : m_errorMessage(std::move(msg)), m_hasError(true) {
        }

        std::string m_errorMessage;
        bool m_hasError = true;
    };


    class Result
    {
    public:
        Result() = default;

        static Result success() { return {}; }
        static Result failure(std::string msg) { return Result(std::move(msg)); }
        static Result failure(std::string msg, DiagnosticCollector* c, NToken& t, NSourceFile* f) {
            c->error(t.location(f), msg);
            return Result(std::move(msg));
        }

        bool hasError() const { return !m_messages.empty(); }
        const std::vector<std::string>& messages() const { return m_messages; }

        void append(std::string msg) {
            m_messages.push_back(std::move(msg));
        }

        Result& operator<<(const std::string& msg) {
            append(msg);
            return *this;
        }

    private:
        std::vector<std::string> m_messages;

        explicit Result(std::string msg) {
            m_messages.push_back(std::move(msg));
        }
    };


    template<typename T>
    class Expected 
    {
    public:
        Expected(T value)
            : m_value(std::move(value)), m_hasError(false), m_got {false} {
        }
        Expected(Result error)
            : m_result(std::move(error)), m_hasError(error.hasError()), m_got {false} {
            if constexpr (std::is_pointer_v<T>) {
                m_value = nullptr;
            }
        }
        ~Expected() {
            if (m_got) return;
            if constexpr (std::is_pointer_v<T>) {
                if (!m_value) return;
                delete m_value;
            }
        }

        bool hasError() const { return m_hasError; }
        const Result& result() const { return m_result; }
        Result& result() { return m_result; }

        const T& value() const { NE_ASSERT(!m_hasError); return m_value; }
        T& value() { 
            NE_ASSERT(!m_hasError);
            m_got = true;
            return m_value;
        }
        T& operator->() {
            return m_value;
        }

        explicit operator bool() const { return !m_hasError; }

    private:
        T m_value;
        Result m_result;
        bool m_hasError;
        bool m_got;
    };


    template<>
    class Expected<void> 
    {
    public:
        Expected() : m_hasError(false) {}
        Expected(Result r) : m_result(std::move(r)), m_hasError(r.hasError()) {}

        bool hasError() const { return m_hasError; }
        const Result& result() const { return m_result; }
        Result& result() { return m_result; }

        explicit operator bool() const { return !m_hasError; }

    private:
        Result m_result;
        bool m_hasError;
    };

#define CHECK_ERROR(V) do { \
    if (!V) { \
        return V.result(); \
    } \
} while(false)


    template<typename E>
    constexpr auto to_underlying(E e) noexcept {
        return static_cast<std::underlying_type_t<E>>(e);
    }
    constexpr DiagnosticLevel operator|(DiagnosticLevel lhs, DiagnosticLevel rhs) {
        return static_cast<DiagnosticLevel>(to_underlying(lhs) | to_underlying(rhs));
    }
    constexpr DiagnosticLevel operator&(DiagnosticLevel lhs, DiagnosticLevel rhs) {
        return static_cast<DiagnosticLevel>(to_underlying(lhs) & to_underlying(rhs));
    }
    constexpr DiagnosticLevel operator~(DiagnosticLevel v) {
        return static_cast<DiagnosticLevel>(~to_underlying(v));
    }
    inline DiagnosticLevel& operator|=(DiagnosticLevel& lhs, DiagnosticLevel rhs) {
        lhs = lhs | rhs;
        return lhs;
    }
    inline DiagnosticLevel& operator&=(DiagnosticLevel& lhs, DiagnosticLevel rhs) {
        lhs = lhs & rhs;
        return lhs;
    }
}