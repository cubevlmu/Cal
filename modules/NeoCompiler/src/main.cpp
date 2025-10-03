// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "neo/Compiler.hpp"

/// Compiler main entrance
int main(int argc, char** argv) {
    neo::NCompiler cmp {argc, argv};
    return cmp.runCompiler();
}