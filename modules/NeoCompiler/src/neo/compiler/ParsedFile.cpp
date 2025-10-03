// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "ParsedFile.hpp"

#include "neo/ast/Base.hpp"

namespace neo {

    NParsedFile::NParsedFile() {

    }


    void NParsedFile::clearNodes() {
        for (auto* item : Nodes) {
            delete item;
        }
        Nodes.clear();
    }
    
} // namespace neo