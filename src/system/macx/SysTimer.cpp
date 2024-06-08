#ifdef __MACH__

#include "system/SysTimer.hpp"
#include <ctime>
#include <sys/_types/_timespec.h>

namespace cal::platform {

    Timer::Timer() {
        last_tick = getRawTimestamp();
        first_tick = last_tick;
    }


    float Timer::getTimeSinceStart() const {
        return float(double(getRawTimestamp() - first_tick) / double(getFrequency()));
    }


    float Timer::getTimeSinceTick() const {
        return float(double(getRawTimestamp() - last_tick) / double(getFrequency()));
    }


    float Timer::tick() {
        const u64 now = getRawTimestamp();
        const float delta = float(double(now - last_tick) / double(getFrequency()));
        last_tick = now;
        return delta;
    }


    u64 Timer::getFrequency() {
        return 1'000'000'000;
    }


    u64 Timer::getRawTimestamp() {
        timespec tick;
        clock_gettime(CLOCK_REALTIME, &tick);
        return u64(tick.tv_sec) * 1000000000 + u64(tick.tv_nsec);
    }

}

#endif