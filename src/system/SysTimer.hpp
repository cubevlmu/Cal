#pragma once

#include "globals.hpp"

namespace cal::platform {

    struct Timer {
        Timer();

        float tick();
        float getTimeSinceStart() const;
        float getTimeSinceTick() const;

        static u64 getRawTimestamp();
        static u64 getFrequency();

        u64 frequency;
        u64 last_tick;
        u64 first_tick;
    };
}