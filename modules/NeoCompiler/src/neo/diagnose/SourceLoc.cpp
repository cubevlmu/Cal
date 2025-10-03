// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "SourceLoc.hpp"

#include "neo/compiler/SourceFile.hpp"

#include <nbase/base/Serializer.hpp>

#include <filesystem>
#include <sstream>

namespace neo {

    std::string SourceLoc::toString() const
    {
        std::stringstream output {};
        std::filesystem::path p(file == nullptr ? "Unknown Source" : file->getPath());
        output << p.filename() << " [" << line << ':' << column << ']';

        return output.str();
    }

    void SourceLoc::write(NSerializer* s) const {
        s->write(line);
        s->write(column);
        s->write(file->getPath());
    }

    void SourceLoc::read(NSerializer* s) const {
        s->read((u64)line);
        s->read((u64)column);
        std::string path;
        s->read(path);

        (void)path;
        //TODO use the result
    }

}