#pragma once

#include "globals.hpp"

namespace cal {

    enum class OpCodes : u8 
    {
        OP_LOAD_K,
        OP_ADD, OP_MINUS, OP_DIV, OP_MULTI,
    };
}