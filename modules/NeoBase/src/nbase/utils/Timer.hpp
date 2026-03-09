/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <chrono>

namespace neo {

    /// Simple timer for calculate process time
    class NTimer {
    public:
	    NTimer() {
		    reset();
	    }

	    void reset() {
		    m_start = clock::now();
		    m_end = m_start;
		    m_stopped = false;
	    }

	    void stop() {
		    m_end = clock::now();
		    m_stopped = true;
	    }

	    i64 nanoTime() const {
		    return to_ns(current() - m_start);
	    }

	    i64 microTime() const {
		    return to_us(current() - m_start);
	    }

	    i64 milliTime() const {
		    return to_ms(current() - m_start);
	    }

	    i64 secondTime() const {
		    return to_s(current() - m_start);
	    }

    private:
	    using clock = std::chrono::steady_clock;

	    static i64 to_ns(auto d) { return std::chrono::duration_cast<std::chrono::nanoseconds>(d).count(); }
	    static i64 to_us(auto d) { return std::chrono::duration_cast<std::chrono::microseconds>(d).count(); }
	    static i64 to_ms(auto d) { return std::chrono::duration_cast<std::chrono::milliseconds>(d).count(); }
	    static i64 to_s (auto d) { return std::chrono::duration_cast<std::chrono::seconds>(d).count(); }

	    clock::time_point current() const {
		    return m_stopped ? m_end : clock::now();
	    }

	    clock::time_point m_start;
	    clock::time_point m_end;
	    bool m_stopped = false;
    };

}