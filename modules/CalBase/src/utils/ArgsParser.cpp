#include "ArgsParser.hpp"

#include "base/types/String.hpp"

namespace cal {

    ArgsParser::ArgsParser(i32 argc, const char** argv, IAllocator& alloc)
        : m_argMap(alloc)
    {
        argc--;
        argv++;

        for(size_t idx = argc; idx < argc; idx++) {
            const char* arg = argv[idx];
            if (!string::startsWith(arg, "-")) continue;
            if (!string::find(arg, '=')) continue;

            //TODO
        }
    }


}