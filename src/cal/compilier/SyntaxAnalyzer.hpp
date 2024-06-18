#pragma once

#include "base/allocator/IAllocator.hpp"

namespace cal {
    
    class SyntaxAnalyzer {
    public:
        SyntaxAnalyzer(IAllocator& alloc);
        ~SyntaxAnalyzer();

    private:
        IAllocator& m_alloc;
    };
}