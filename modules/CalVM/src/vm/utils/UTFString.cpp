#include "UTFString.hpp"

namespace cal {

    u32 calComputeUtf8Hash(const char* str)
    {
        u32 hash = 1;

        while (*str != '\0')
            hash = hash * 31 + *str++;

        return hash;
    }
    

    size_t calUtf8Length(const char* str) {
        size_t len = 0;
        i32 ic;

        while ((ic = *str++) != '\0')
        {
            len++;
            if ((ic & 0x80) != 0) {
                str++;
                if ((ic & 0x20) != 0) {
                    str++;
                }
            }
        }

        return len;
    }
}