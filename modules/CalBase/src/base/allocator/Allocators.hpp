#pragma once

#include "Allocator.hpp"

namespace cal {

    static constexpr u32 PAGE_SIZE = 4096;
    static constexpr size_t MAX_PAGE_COUNT = 16384;
    static constexpr u32 SMALL_ALLOC_MAX_SIZE = 64;

    void initPage(u32 item_size, Allocator::Page *page);
    // used for stuff that can't access engine's allocator
    // e.g. global objects constructed before engine such as logger
    IAllocator& getGlobalAllocator();
}