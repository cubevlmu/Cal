#include "system/StdLibExt.hpp"

#include <stdlib.h>

namespace cal::platform {

    // Replace Memory Alloc Method On MacOS
    // Reason : Apple Change The Implenment Of 'aligned_alloc'.
    // Use 'malloc' On MacOS 'aligned_alloc' On Other Platform
    // Conclusion : Fuck APPLE
    void* toy_aligned_alloc(size_t size, size_t alignment) {
        #ifdef __MACH__
        void* ptr = malloc(size + alignment - 1);

        if (ptr) {
            // Calculate The Alignment
            void* aligned_ptr = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
            *reinterpret_cast<void**>(aligned_ptr) = ptr;

            return aligned_ptr;
        }
        else {
            return nullptr;
        }
        #else
        aligned_alloc(size, alignment);
        #endif
    }
}