/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include "SourceLoc.hpp"
#include "neo/compiler/SourceFile.hpp"
#include "neo/compiler/Tokens.hpp"

#include <nbase/base/Assert.hpp>
#include <type_traits>

namespace neo {

    enum class DiagnosticLevel : u32 {
        kNone = 0,
        kError = 1 << 0,
        kWarning = 1 << 1,
        kNote = 1 << 2,
        kHint = 1 << 3,
    };


    struct Diagnostic {
        DiagnosticLevel level;
        SourceLoc location;
        String message;
    };


    class DiagnosticCollector
    {
    public:
        DiagnosticCollector() = default;

    public:
        void report(DiagnosticLevel level, const SourceLoc& loc, const String& message);

        NE_FORCE_INLINE void error(const SourceLoc& loc, const String& msg) {
            report(DiagnosticLevel::kError, loc, msg);
        }
        NE_FORCE_INLINE void warning(const SourceLoc& loc, const String& msg) {
            report(DiagnosticLevel::kWarning, loc, msg);
        }
        NE_FORCE_INLINE void note(const SourceLoc& loc, const String& msg) {
            report(DiagnosticLevel::kNote, loc, msg);
        }
        NE_FORCE_INLINE void hint(const SourceLoc& loc, const String& msg) {
            report(DiagnosticLevel::kHint, loc, msg);
        }

        NE_FORCE_INLINE bool hasError() const { return m_errorCount > 0; }
        NE_FORCE_INLINE int getErrorCount() const { return m_errorCount; }
        const Vector<Diagnostic>& diagnostics() const { return m_diagnostics; }

        void printAll() const;
        void clear(DiagnosticLevel flags = DiagnosticLevel::kNone);

    private:
        void printOne(const Diagnostic& diagnostic) const;

    private:
        Vector<Diagnostic> m_diagnostics;
        int m_errorCount = 0;
    };


    template<typename T>
    class ErrorOr {
    public:
        static ErrorOr<T> success(T value) {
            return ErrorOr(std::move(value));
        }

        static ErrorOr<T> failure(String errorMsg) {
            return ErrorOr(std::move(errorMsg));
        }

        bool hasError() const { return m_hasError; }
        const String& errorMessage() const { return m_errorMessage; }
        const T& value() const { NE_ASSERT(!m_hasError); return m_value; }
        T& value() { NE_ASSERT(!m_hasError); return m_value; }

        explicit operator bool() const { return !m_hasError; }

    private:
        ErrorOr(T val)
            : m_value(std::move(val)), m_hasError(false) {
        }

        ErrorOr(String msg)
            : m_errorMessage(std::move(msg)), m_hasError(true) {
        }

        T m_value;
        String m_errorMessage;
        bool m_hasError = true;
    };


    template<>
    class ErrorOr<void> {
    public:
        static ErrorOr<void> success() {
            return {};
        }

        static ErrorOr<void> failure(String errorMsg) {
            return {std::move(errorMsg)};
        }

        bool hasError() const { return m_hasError; }
        const String& errorMessage() const { return m_errorMessage; }

        explicit operator bool() const { return !m_hasError; }

    private:
        ErrorOr() : m_hasError(false) {}
        ErrorOr(String msg)
            : m_errorMessage(std::move(msg)), m_hasError(true) {
        }

        String m_errorMessage;
        bool m_hasError = true;
    };


	/// Result of compiler function's status
    class Result
    {
    public:
        Result() = default;

        static Result success() { return Result{"", true}; }
        static Result failure(String msg) { return Result(std::move(msg), false); }
        static Result failure(String msg, DiagnosticCollector* c, const NToken& t, NSourceFile* f) {
            c->error(t.location(f), msg);
            return Result(std::move(msg), false);
        }

        bool hasError() const { return !m_isOk; }

    private:
		bool m_isOk = false;
		String m_msg = "";

        explicit Result(String msg, bool isOk = false)
		    : m_msg {std::move(msg)}
			, m_isOk {isOk}
		{}
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


	template<typename T>
	class ExpectedSafe
	{
	public:
		ExpectedSafe(T value)
			: m_value(std::move(value))
			, m_hasError(false)
		{}

		ExpectedSafe(Result error)
			: m_result(std::move(error))
			, m_hasError(error.hasError())
		{}

		ExpectedSafe(const ExpectedSafe&) = delete;
		ExpectedSafe& operator=(const ExpectedSafe&) = delete;

		ExpectedSafe(ExpectedSafe&& other) noexcept
			: m_value(std::move(other.m_value))
			, m_result(std::move(other.m_result))
			, m_hasError(other.m_hasError)
		{}

		ExpectedSafe& operator=(ExpectedSafe&& other) noexcept {
			if (this != &other)
			{
				m_value = std::move(other.m_value);
				m_result = std::move(other.m_result);
				m_hasError = other.m_hasError;
			}
			return *this;
		}
		~ExpectedSafe() = default;

	public:
		bool hasError() const { return m_hasError; }

		const Result& result() const { return m_result; }
		Result& result() { return m_result; }

		const T& value() const {
			NE_ASSERT(!m_hasError);
			return m_value;
		}

		T& value() {
			NE_ASSERT(!m_hasError);
			return m_value;
		}

		auto operator->() {
			NE_ASSERT(!m_hasError);
			return m_value.operator->();
		}

		explicit operator bool() const { return !m_hasError; }

	private:
		T       m_value{};
		Result  m_result{};
		bool    m_hasError = false;
	};


	template<>
	class ExpectedSafe<void>
	{
	public:
		ExpectedSafe()
			: m_hasError(false)
		{}

		ExpectedSafe(Result error)
			: m_result(std::move(error))
			, m_hasError(error.hasError())
		{}

		ExpectedSafe(const ExpectedSafe&) = delete;
		ExpectedSafe& operator=(const ExpectedSafe&) = delete;

		ExpectedSafe(ExpectedSafe&& other) noexcept
			: m_result(std::move(other.m_result))
			, m_hasError(other.m_hasError)
		{}

		ExpectedSafe& operator=(ExpectedSafe&& other) noexcept
		{
			if (this != &other)
			{
				m_result = std::move(other.m_result);
				m_hasError = other.m_hasError;
			}
			return *this;
		}

		~ExpectedSafe() = default;

	public:
		bool hasError() const { return m_hasError; }

		const Result& result() const { return m_result; }
		Result& result() { return m_result; }
		explicit operator bool() const { return !m_hasError; }

	private:
		Result m_result {};
		bool   m_hasError = false;
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
