// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>

#include <string>
#include <sstream>
#include <array>
#include <vector>

namespace neo {

#define CONCAT_STR(NAME, ...) constexpr const char* NAME = ::neo::concatStr(__VA_ARGS__)

    void replaceAll(std::string& str, const char* from, const char* to);
    void splitStr(std::vector<std::string>& out, const std::string& str, const char* delim, bool skipEmpty = true);
    void splitStr(std::vector<std::string>& out, const std::string& str, char delim, bool skipEmpty = true);
    void splitAt(std::vector<std::string>& out, int pos, const std::string& str);
    int findLast(const std::string& str, char hint);
    
    template <typename... T>
    std::string msg(T&&... args) {
        std::ostringstream oss {};
        (oss << ... << args);
        return oss.str();
    }

    template <typename... T>
    std::string concatStr(const T&... args) {
        std::string r;
        auto append = [](std::string* s, const char* v) {
            s->append(v);
        };
        (append(&r, args), ...);
        return r;
    }
}