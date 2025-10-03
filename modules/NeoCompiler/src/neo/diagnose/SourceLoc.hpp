// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <string>

namespace neo {

    struct SourceLoc
    {
        psize line;
        psize column;
        class NSourceFile* file;

        std::string toString() const;
        void write(class NSerializer*) const;
        void read(class NSerializer*) const;
    };
    
}