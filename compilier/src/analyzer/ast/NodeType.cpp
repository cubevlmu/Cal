#include "NodeType.hpp"

namespace cal {

    static const char* ENUM_STR[] = {
        "TypeNode", "TypeNumber"
    };


    const char* getString(ASTTypes type)
    {
        return ENUM_STR[(i32)type];
    }
    
} // namespace cal