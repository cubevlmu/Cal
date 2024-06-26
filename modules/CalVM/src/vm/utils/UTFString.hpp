#pragma once

#include "globals.hpp"

namespace cal {

    u32 calComputeUtf8Hash(const char* str);
    size_t calUtf8Length(const char* str);

}