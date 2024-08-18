#include <iostream>

#include <globals.hpp>
#include <base/Logger.hpp>

namespace cal {
    i32 compilier_main(i32 argc, char** argv) {
        InitLogger();

        LogDebug("Hello world!");

        return 0;
    }
}


int main(int argc, char** argv) {
    return cal::compilier_main(argc, argv);
}