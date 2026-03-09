/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Memory.hpp"

#include "nbase/base/Assert.hpp"

#if NE_USE_RPMALLOC
#include <rpmalloc/rpmalloc.h>
#else
#include <memory>
#endif

#include <cstddef>

namespace neo {

#if NE_USE_RPMALLOC
	typedef u16 offset_t;
#define align_mem_up(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

    void *alloc(psize size) {
        return allocAligned(size, alignof(std::max_align_t));
    }


    void *allocAligned(psize size, psize alignment) {
        void* ptr = nullptr;

        // Alignment always has to be power of two
        NE_ASSERT((alignment & (alignment - 1)) == 0);

        if (alignment && size)
        {
            u32 pad = sizeof(offset_t) + (alignment - 1);
            void* p = rpmalloc(size + pad);
            if (p)
            {
                // Add the offset size to malloc's pointer
                ptr = (void*)align_mem_up(((uintptr_t)p + sizeof(offset_t)), alignment);

                // Calculate the offset and store it behind aligned pointer
                *((offset_t*)ptr - 1) = (offset_t)((uintptr_t)ptr - (uintptr_t)p);
            }
        }
        return ptr;
    }


    void free(void* ptr) {
        if (!ptr) return;

        // Walk backwards from the passed-in pointer to get the pointer offset
        offset_t offset = *((offset_t*)ptr - 1);

        // Get original pointer
        void* p = (void*)((u8*)ptr - offset);

        // Free memory
        rpfree(p);
    }

#else

	typedef u16 offset_t;
#define align_mem_up(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

	void *alloc(psize size) {
		return allocAligned(size, alignof(std::max_align_t));
	}


	void *allocAligned(psize size, psize alignment) {
		void* ptr = nullptr;

		// Alignment always has to be power of two
		NE_ASSERT((alignment & (alignment - 1)) == 0);

		if (alignment && size)
		{
			u32 pad = sizeof(offset_t) + (alignment - 1);
			void* p = malloc(size + pad);
			if (p)
			{
				// Add the offset size to malloc's pointer
				ptr = (void*)align_mem_up(((uintptr_t)p + sizeof(offset_t)), alignment);

				// Calculate the offset and store it behind aligned pointer
				*((offset_t*)ptr - 1) = (offset_t)((uintptr_t)ptr - (uintptr_t)p);
			}
		}
		return ptr;
	}


	void free(void* ptr) {
		if (!ptr) return;

		// Walk backwards from the passed-in pointer to get the pointer offset
		offset_t offset = *((offset_t*)ptr - 1);

		// Get original pointer
		void* p = (void*)((u8*)ptr - offset);

		// Free memory
		::free(p);
	}

#endif

}
