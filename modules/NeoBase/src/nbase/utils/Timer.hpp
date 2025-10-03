// Created by cubevlmu on 2025/8/16.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <chrono>

namespace neo {

    /// Simple timer for calculate process time
    class NTimer
    {
    public:
        NTimer() : m_start {std::chrono::steady_clock::now()}, m_end {} {
        }
        ~NTimer() = default;

        void end() {
            m_end = std::chrono::steady_clock::now();
        }
        void reset() {
            m_start = std::chrono::steady_clock::now();
            m_end = {};
        }
        i64 nanoTime() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(m_end - m_start).count();
        }
        i64 milliTime() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_start).count();
        }
        i64 secondTime() const {
            return std::chrono::duration_cast<std::chrono::seconds>(m_end - m_start).count();
        }

    private:
        std::chrono::steady_clock::time_point m_start;
        std::chrono::steady_clock::time_point m_end;
    };
}