#pragma once

#include "base/allocator/IAllocator.hpp"
#include "globals.hpp"
#include "base/types/container/HashMap.hpp"

namespace cal {

    class ArgsParser 
    {
    public:
        ArgsParser(i32 argc, const char** argv, IAllocator& alloc);

        std::string getArg(const std::string& key, const std::string& defaultVal) const;
        std::string getArg(const std::string& key) const;

    private:
        HashMap<std::string, std::string> m_argMap;
    };
}