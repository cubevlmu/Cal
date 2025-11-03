// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>

#include <nbase/base/Assert.hpp>
#include <nbase/base/Logger.hpp>

#include <string_view>
#include <fstream>

namespace neo {

    /// Customized serializable component interface
    struct ISerializable {
        virtual ~ISerializable() {};
        virtual void write(class NSerializer* s) { (void)s; };
        virtual void read(class NSerializer* s) { (void)s; };
    };


    /// Basic serializer interface for reading & writting
    class NSerializer
    {
    public:
        virtual ~NSerializer() {}

    public:
        virtual void write(void* data, psize size) = 0;
        virtual void* read(psize size) = 0;
        virtual psize read(void*& ptr, psize size) = 0;
        virtual psize getPos() = 0;
        virtual void setPos(psize pos) = 0;

    public:
        NE_FORCE_INLINE void write(const char* str) {
            write((psize)strlen(str));
            write((void*)str, sizeof(char) * strlen(str));
        }
        NE_FORCE_INLINE void write(const std::string_view str) {
            write((psize)str.length());
            write((void*)str.data(), sizeof(char) * str.length());
        }
        NE_FORCE_INLINE void write(const std::string& str) {
            write((psize)str.length());
            write((void*)str.c_str(), sizeof(char) * str.length());
        }
        NE_FORCE_INLINE void write(bool v) {
            write((void*)&v, 1);
        }
        NE_FORCE_INLINE void write(i8 v) {
            write((void*)&v, sizeof(i8));
        }
        NE_FORCE_INLINE void write(i16 v) {
            write((void*)&v, sizeof(i16));
        }
        NE_FORCE_INLINE void write(i32 v) {
            write((void*)&v, sizeof(v));
        }
        NE_FORCE_INLINE void write(i64 v) {
            write((void*)&v, sizeof(i64));
        }
        NE_FORCE_INLINE void write(f32 v) {
            write((void*)&v, sizeof(f32));
        }
        NE_FORCE_INLINE void write(f64 v) {
            write((void*)&v, sizeof(f64));
        }
        NE_FORCE_INLINE void write(psize s) {
            write((void*)&s, sizeof(psize));
        }
        NE_FORCE_INLINE void write(ISerializable* s) {
            s->write(this);
        }

    public:
        void read(std::string& v) {
            psize len = 0;
            read((psize)len);
            v.resize(len);
            void* vptr = v.data();
            read(vptr, len * sizeof(char));
        }
        NE_FORCE_INLINE void read(bool& v) {
            void* vptr = &v;
            read(vptr, sizeof(bool));
        }
        NE_FORCE_INLINE void read(i8& v) {
            void* vptr = &v;
            read(vptr, sizeof(i8));
        }
        NE_FORCE_INLINE void read(i16& v) {
            void* vptr = &v;
            read(vptr, sizeof(i16));
        }
        NE_FORCE_INLINE void read(i32& v) {
            void* vptr = &v;
            read(vptr, sizeof(i32));
        }
        NE_FORCE_INLINE void read(i64& v) {
            void* vptr = &v;
            read(vptr, sizeof(i64));
        }
        NE_FORCE_INLINE void read(f32& v) {
            void* vptr = &v;
            read(vptr, sizeof(f32));
        }
        NE_FORCE_INLINE void read(f64& v) {
            void* vptr = &v;
            read(vptr, sizeof(f64));
        }
        NE_FORCE_INLINE void read(psize& v) {
            void* vptr = &v;
            read(vptr, sizeof(psize));
        }
        NE_FORCE_INLINE void read(ISerializable* s) {
            s->read(this);
        }
    };


    /// File based input serializer for reading from file stream
    class IFileSerializer final : public NSerializer
    {
    public:
        explicit IFileSerializer(const char* path);
        ~IFileSerializer() override;

    public:
        void write(void* data, psize size) override {
            (void)data; (void)size;
            LogError("[ISGSerializer] ReadOnly Serializer");
            NE_ASSERT(false);
        }
        void* read(psize size) override;
        psize read(void*& ptr, psize size) override;
        NE_FORCE_INLINE psize getPos() override {
            return m_stream.tellg();
        }
        NE_FORCE_INLINE void setPos(psize pos) override {
            m_stream.seekg((i64)pos, std::ios::beg);
        }

    private:
        std::ifstream m_stream;
    };


    /// File based output serializer for writting to file stream
    class OFileSerializer final : public NSerializer
    {
    public:
        explicit OFileSerializer(const char* path);
        ~OFileSerializer() override;

    public:
        void write(void* data, psize size) override;
        void* read(psize size) override {
            (void)size;
            LogError("[OSGSerializer] WriteOnly Serializer");
            NE_ASSERT(false);
            return nullptr;
        }
        psize read(void*& ptr, psize size) override {
            (void)ptr;
            LogError("[OSGSerializer] WriteOnly Serializer");
            NE_ASSERT(false);
            return -1;
        }
        NE_FORCE_INLINE psize getPos() noexcept override {
            return m_stream.tellp();
        }
        NE_FORCE_INLINE void setPos(psize pos) override {
            m_stream.seekp((i64)pos, std::ios::beg);
        }

    private:
        std::ofstream m_stream;
    };
}
