// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "StringUtils.hpp"

namespace neo {

    void replaceAll(std::string& str, const char* from, const char* to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, strlen(from), to);
            start_pos += strlen(to);
        }
    }

    void splitStr(std::vector<std::string>& out, const std::string& str, char delim, bool skipEmpty) {
        size_t start = 0;
        size_t pos = 0;

        while ((pos = str.find(delim, start)) != std::string::npos) {
            if (pos > start || !skipEmpty) {
                out.emplace_back(str.substr(start, pos - start));
            }
            start = pos + 1;
        }

        if (start < str.size() || !skipEmpty) {
            out.emplace_back(str.substr(start));
        }
    }

    void splitStr(std::vector<std::string>& out, const std::string& str, const char* delim, bool skipEmpty) {
        size_t start = 0;
        size_t pos = 0;
        size_t dlen = std::strlen(delim);

        while ((pos = str.find(delim, start)) != std::string::npos) {
            if (pos > start || !skipEmpty) {
                out.emplace_back(str.substr(start, pos - start));
            }
            start = pos + dlen;
        }

        if (start < str.size() || !skipEmpty) {
            out.emplace_back(str.substr(start));
        }
    }

    void splitAt(std::vector<std::string>& out, int pos, const std::string& str) {
        out.clear();
        if (pos < 0 || pos > static_cast<int>(str.size())) {
            out.push_back(str);
            return;
        }
        out.push_back(str.substr(0, pos));
        out.push_back(str.substr(pos));
    }

    int findLast(const std::string& str, char hint) {
        auto pos = str.find_last_of(hint);
        if (pos == std::string::npos) return -1;
        return static_cast<int>(pos);
    }
}