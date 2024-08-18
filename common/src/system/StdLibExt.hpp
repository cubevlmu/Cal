#pragma once

#ifdef __MACH__
#include <sys/_types/_size_t.h>
#endif

// SO DO YOU KNOW WHY THIS FILE COMING HERE???
// FUCK APPLE FUCK MACOS
// EDITING STDLIB POSIX APIS BY THEM SELF

namespace cal::platform {
    void* toy_aligned_alloc(size_t, size_t);
}