#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace cal {

    inline std::string readLine() {
        std::cin.clear();
        std::string val;
        std::getline(std::cin, val);
        return val;
    }

    inline void waitAnyKeyToExit() {
        readLine();
    }

#define BeginAppender() std::stringstream _ss
#define AppenderAppend(str) _ss << (str)
#define EndAppender() _ss.str()
}