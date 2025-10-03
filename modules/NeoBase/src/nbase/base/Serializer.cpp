// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Serializer.hpp"

#include <filesystem>

namespace neo {

    IFileSerializer::IFileSerializer(const char* path)
        : m_stream {}
    {
        if (!std::filesystem::exists(path)) {
            LogError("[IFileSerializer] File not found -> {}", path);
            NE_ASSERT(false);
        }

        m_stream.open(path, std::ios::in | std::ios::binary);
        if (!m_stream.is_open()) {
            LogError("[IFileSerializer] File can't be read -> {0}", path);
            NE_ASSERT(false);
        }
    }

    IFileSerializer::~IFileSerializer() {
        m_stream.close();
    }

    void* IFileSerializer::read(psize size) {
        char* ptr = (char*)malloc(size * sizeof(char));
        m_stream.read(ptr, (i64)size);
        return ptr;
    }

    u64 IFileSerializer::read(void*& ptr, psize size) {
        m_stream.read((char*)ptr, (i64)size);
        return size;
    }


    OFileSerializer::OFileSerializer(const char* path)
        : m_stream {}
    {
        m_stream.open(path, std::ios::out | std::ios::binary);
        if (!m_stream.is_open()) {
            LogError("[OSGSerializer] File can't be write -> {0}", path);
            NE_ASSERT(false);
        }
    }

    OFileSerializer::~OFileSerializer() {
        m_stream.flush();
        m_stream.close();
    }

    void OFileSerializer::write(void* data, psize size) {
        m_stream.write((char*)data, (i64)size);
    }


} // namespace neo