#include "Allocators.hpp"


namespace cal {

    u32 sizeToBin(size_t n) {
        ASSERT(n > 0);
        ASSERT(n <= SMALL_ALLOC_MAX_SIZE);
#ifdef _WIN32
        unsigned long res;
        return _BitScanReverse(&res, ((unsigned long)n - 1) >> 2) ? res : 0;
#else
        size_t tmp = (n - 1) >> 2;
        auto res = tmp == 0 ? 0 : 31 - __builtin_clz(tmp);
        ASSERT(res <= sizeof(((Allocator*)nullptr)->m_free_lists)); //TODO
        return res;
#endif
    }

    static Allocator* alloc = nullptr;

    IAllocator& getGlobalAllocator() {
        if(alloc == nullptr) alloc = new Allocator();
        
        return *alloc;
    }

}