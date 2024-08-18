#pragma once

#include "globals.hpp"

namespace cal {

    enum ASTTypes : i8 {
        TYPE_NODE, TYPE_NUMBER
    };

    const char* getString(ASTTypes type);
}