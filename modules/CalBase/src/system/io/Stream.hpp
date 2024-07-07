#pragma once

#include "base/types/Array.hpp"

namespace cal {

    struct CAL_API IOStream {
        virtual bool write(const void* buffer, u64 size) = 0;

        IOStream& operator << (struct StringView str);
        IOStream& operator << (u64 value);
        IOStream& operator << (i64 value);
        IOStream& operator << (i32 value);
        IOStream& operator << (u32 value);
        IOStream& operator << (float value);
        IOStream& operator << (double value);
        template <typename T> bool write(const T& value);
        template <typename T> bool writeArray(const Array<T>& value);
    };


    struct CAL_API IIStream {
        virtual bool read(void* buffer, u64 size) = 0;
        virtual u64 size() const = 0;

        template <typename T> void read(T& value) { read(&value, sizeof(T)); }
        template <typename T> T read();
        template <typename T> void readArray(Array<T>* array);
    };


    struct CAL_API PagedOStream final : IOStream {
        friend struct PagedIStream;

        PagedOStream(struct PageAllocator& allocator);
        ~PagedOStream();
        bool write(const void* buffer, u64 size) override;

        using IOStream::write;

        struct Page {
            Page* next = nullptr;
            u32 size = 0;
            u8 data[4096 - sizeof(next) - sizeof(size)];
        };

    private:
        Span<u8> reserve(u32 size);
        PageAllocator& m_allocator;
        Page* m_head;
        Page* m_tail;
    };
    

    struct CAL_API PagedIStream final : IIStream {
        PagedIStream(const PagedOStream& src);
        bool read(void* buffer, u64 size) override;

        u64 size() const override { ASSERT(false); return 0; }
        bool isEnd() const { return !m_page || (!m_page->next && m_page_pos == m_page->size); }

        using IIStream::read;

    private:
        const PagedOStream::Page* m_page = nullptr;
        u32 m_page_pos = 0;
    };


    struct CAL_API MemoryOStream final : IOStream {
        explicit MemoryOStream(struct IAllocator& allocator);
        MemoryOStream(void* data, u64 size);
        MemoryOStream(MemoryOStream&& rhs);
        MemoryOStream(const MemoryOStream& rhs);
        MemoryOStream(const MemoryOStream& blob, IAllocator& allocator);
        MemoryOStream(const struct MemoryIStream& blob, IAllocator& allocator);
        ~MemoryOStream();
        void operator =(const MemoryOStream& rhs);
        void operator =(MemoryOStream&& rhs);
        u8 operator[](u32 idx) const;
        u8& operator[](u32 idx);

        bool write(const void* data, u64 size) override;
        bool writeString(const std::string& data);
        bool writeString(const char* data);

        Span<u8> releaseOwnership();
        operator Span<const u8>() const { return Span(m_data, m_data + m_size); }
        void resize(u64 size);
        void reserve(u64 size);
        const u8* data() const { return m_data; }
        u8* getMutableData() { return m_data; }
        u64 capacity() const { return m_capacity; }
        u64 size() const { return m_size; }
        void write(const struct String& string);
        void writeString(StringView string);
        template <typename T> void write(const T& value);
        void clear();
        void* skip(u64 size);
        bool empty() const { return m_size == 0; }
        void free();
        IAllocator& getAllocator() { return *m_allocator; }

    private:
        u8* m_data;
        u64 m_capacity;
        u64 m_size;
        IAllocator* m_allocator;
    };


    struct CAL_API MemoryIStream final : IIStream {
        MemoryIStream(const void* data, u64 size);
        MemoryIStream(Span<const u8> data);
        explicit MemoryIStream(const MemoryOStream& blob);

        void set(const void* data, u64 size);
        bool read(void* data, u64 size) override;
        bool readString(std::string& str);
        bool readString(char*& str);

        bool read(String& string);
        const void* skip(u64 size);
        const void* getData() const { return m_data; }
        u64 size() const override { return m_size; }
        u64 remaining() const { return m_size - m_pos; }
        u64 getPosition() const { return m_pos; }
        void setPosition(u64 pos) { m_pos = pos; }
        const char* readString();
        bool hasOverflow() const { return m_has_overflow; }

        template <typename T>
        T getAs() const {
            ASSERT(m_pos + sizeof(T) < m_size);
            return *(T*)(m_data + m_pos);
        }

        using IIStream::read;
    private:
        bool m_has_overflow = false;
        const u8* m_data;
        u64 m_size;
        u64 m_pos = 0;
    };

    template <typename T> void IIStream::readArray(Array<T>* array) {
        const i32 size = read<i32>();
        array->resize(size);
        read(array->begin(), array->byte_size());
    }

    template <typename T> T IIStream::read()
    {
        T v;
        read(&v, sizeof(v));
        return v;
    }

    template <> inline bool IIStream::read<bool>()
    {
        u8 v;
        read(&v, sizeof(v));
        return v != 0;
    }

    template <typename T> bool IOStream::writeArray(const Array<T>& value) {
        return write(value.size()) && write(value.begin(), value.byte_size());
    }

    template <typename T> bool IOStream::write(const T& value)
    {
        return write(&value, sizeof(T));
    }

    template <typename T> void MemoryOStream::write(const T& value)
    {
        write(&value, sizeof(T));
    }

    template <> inline void MemoryOStream::write<bool>(const bool& value)
    {
        u8 v = value;
        write(&v, sizeof(v));
    }

}