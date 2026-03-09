/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "SourceLoc.hpp"

#include "neo/compiler/SourceFile.hpp"

#include <nbase/base/Serializer.hpp>
#include "nbase/base/Format.hpp"

namespace neo {

    String SourceLoc::path() const
    {
        if (file == nullptr) {
            return "Unknown Source";
        }
        return file->getPath();
    }

    String SourceLoc::toString() const
    {
        return neo::format("{}:{}:{}", path(), line, column);
    }

    void SourceLoc::write(NSerializer* s) const {
        s->write(line);
        s->write(column);
        s->write(file == nullptr ? StringView{} : StringView(file->getPath()));
    }

    void SourceLoc::read(NSerializer* s) {
        void* linePtr = &line;
        s->read(linePtr, sizeof(line));

        void* columnPtr = &column;
        s->read(columnPtr, sizeof(column));

        std::string path;
        s->read(path);
        file = nullptr;

        (void)path;
    }

}
