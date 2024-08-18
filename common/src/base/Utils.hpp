#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace Json {
    class Value;
}

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

    namespace utils {
        std::string buildOutputFromJson(const Json::Value& val, bool compact = true);
        bool        writeOutputFromJson(const Json::Value& val, const std::string& fn = "ParserResult.json", bool compact = true);
    }

#define BeginAppender() std::stringstream _ss
#define AppenderAppend(str) _ss << (str)
#define EndAppender() _ss.str()
}