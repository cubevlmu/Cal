#include "Stream.hpp"

#include "base/math/Math.hpp"
#include "base/allocator/PageAllocator.hpp"
#include "base/types/String.hpp"

namespace cal {

    MemoryOStream::MemoryOStream(IAllocator& allocator)
        : m_allocator(&allocator)
        , m_data(nullptr)
        , m_capacity(0)
        , m_size(0)
    {}


    MemoryOStream::MemoryOStream(void* data, u64 size)
        : m_data((u8*)data)
        , m_capacity(size)
        , m_allocator(nullptr)
        , m_size(0)
    {
    }


    MemoryOStream::MemoryOStream(const MemoryOStream& blob, IAllocator& allocator)
        : m_allocator(&allocator)
        , m_size(blob.m_size)
    {
        if (blob.m_capacity > 0)
        {
            m_data = (u8*)allocator.allocate(blob.m_capacity, 1);
            memcpy(m_data, blob.m_data, blob.m_capacity);
            m_capacity = blob.m_capacity;
        }
        else
        {
            m_data = nullptr;
            m_capacity = 0;
        }
    }


    MemoryOStream::MemoryOStream(const MemoryIStream& blob, IAllocator& allocator)
        : m_allocator(&allocator)
        , m_size(blob.size())
    {
        if (blob.size() > 0)
        {
            m_data = (u8*)allocator.allocate(blob.size(), 1);
            memcpy(m_data, blob.getData(), blob.size());
            m_capacity = blob.size();
        }
        else
        {
            m_data = nullptr;
            m_capacity = 0;
        }
    }


    MemoryOStream::~MemoryOStream()
    {
        if (m_allocator) m_allocator->deallocate(m_data);
    }


    IOStream& IOStream::operator << (StringView str) {
        write(str.begin, str.size());
        return *this;
    }

    IOStream& IOStream::operator << (i32 value)
    {
        char tmp[20];
        string::toCString(value, Span(tmp));
        write(tmp, string::stringLength(tmp));
        return *this;
    }


    IOStream& IOStream::operator << (u64 value)
    {
        char tmp[40];
        string::toCString(value, Span(tmp));
        write(tmp, string::stringLength(tmp));
        return *this;
    }


    IOStream& IOStream::operator << (i64 value)
    {
        char tmp[40];
        string::toCString(value, Span(tmp));
        write(tmp, string::stringLength(tmp));
        return *this;
    }


    IOStream& IOStream::operator << (u32 value)
    {
        char tmp[20];
        string::toCString(value, Span(tmp));
        write(tmp, string::stringLength(tmp));
        return *this;
    }


    IOStream& IOStream::operator << (float value)
    {
        char tmp[30];
        string::toCString(value, Span(tmp), 6);
        write(tmp, string::stringLength(tmp));
        return *this;
    }


    IOStream& IOStream::operator << (double value)
    {
        char tmp[40];
        string::toCString(value, Span(tmp), 12);
        write(tmp, string::stringLength(tmp));
        return *this;
    }


    MemoryOStream::MemoryOStream(MemoryOStream&& rhs)
    {
        m_allocator = rhs.m_allocator;
        m_size = rhs.m_size;
        m_capacity = rhs.m_capacity;
        m_data = rhs.m_data;

        rhs.m_data = nullptr;
        rhs.m_capacity = 0;
        rhs.m_size = 0;
    }



    MemoryOStream::MemoryOStream(const MemoryOStream& rhs)
    {
        m_allocator = rhs.m_allocator;
        m_size = rhs.m_size;
        if (rhs.m_capacity > 0)
        {
            m_data = (u8*)m_allocator->allocate(rhs.m_capacity, 1);
            memcpy(m_data, rhs.m_data, rhs.m_capacity);
            m_capacity = rhs.m_capacity;
        }
        else
        {
            m_data = nullptr;
            m_capacity = 0;
        }
    }


    void MemoryOStream::operator =(const MemoryOStream& rhs)
    {
        ASSERT(rhs.m_allocator);
        if (m_allocator) m_allocator->deallocate(m_data);

        m_allocator = rhs.m_allocator;
        m_size = rhs.m_size;
        if (rhs.m_capacity > 0)
        {
            m_data = (u8*)m_allocator->allocate(rhs.m_capacity, 1);
            memcpy(m_data, rhs.m_data, rhs.m_capacity);
            m_capacity = rhs.m_capacity;
        }
        else
        {
            m_data = nullptr;
            m_capacity = 0;
        }
    }


    void MemoryOStream::operator =(MemoryOStream&& rhs)
    {
        ASSERT(rhs.m_allocator);
        if (m_allocator) m_allocator->deallocate(m_data);

        m_allocator = rhs.m_allocator;
        m_size = rhs.m_size;
        m_data = rhs.m_data;
        m_capacity = rhs.m_capacity;

        rhs.m_size = 0;
        rhs.m_capacity = 0;
        rhs.m_data = nullptr;
    }


    void* MemoryOStream::skip(u64 size)
    {
        ASSERT(size > 0 || m_capacity > 0);

        if (m_size + size > m_capacity)
        {
            reserve((m_size + size) << 1);
        }
        void* ret = (u8*)m_data + m_size;
        m_size += size;
        return ret;
    }

    u8 MemoryOStream::operator[](u32 idx) const {
        ASSERT(idx < m_size);
        return m_data[idx];
    }

    u8& MemoryOStream::operator[](u32 idx) {
        ASSERT(idx < m_size);
        return m_data[idx];
    }

    static_assert(sizeof(PagedOStream::Page) == PageAllocator::PAGE_SIZE);


    PagedOStream::PagedOStream(PageAllocator& allocator)
        : m_allocator(allocator)
    {
        m_tail = m_head = new (NewPlaceholder(), m_allocator.allocate(true)) Page;
    }


    PagedOStream::~PagedOStream() {
        Page* p = m_head;
        while (p) {
            Page* tmp = p;
            p = p->next;
            m_allocator.deallocate(tmp, true);
        }
    }


    Span<u8> PagedOStream::reserve(u32 size) {
        if (m_tail->size == lengthOf(m_tail->data)) {
            Page* new_page = new (NewPlaceholder(), m_allocator.allocate(true)) Page;
            m_tail->next = new_page;
            m_tail = new_page;
        }

        u8* res = m_tail->data + m_tail->size;
        size = minimum(size, u32(sizeof(m_tail->data) - m_tail->size));
        m_tail->size += size;
        return Span(res, size);
    }

    bool PagedOStream::write(const void* data, u64 size) {
        const u8* src = (const u8*)data;
        while (size > 0) {
            Span<u8> dst = reserve((u32)size);
            memcpy(dst.begin(), data, dst.length());
            size -= dst.length();
        }
        return true;
    }

    PagedIStream::PagedIStream(const PagedOStream& src)
        : m_page(src.m_head)
    {
    }


    bool PagedIStream::read(void* buffer, u64 size) {
        u8* dst = (u8*)buffer;

        while (size > 0) {
            if (m_page_pos == m_page->size) {
                if (!m_page->next) return false;

                m_page_pos = 0;
                m_page = m_page->next;
            }
            const u32 chunk_size = minimum(u32(size), m_page->size - m_page_pos);
            memcpy(dst, m_page->data + m_page_pos, chunk_size);
            m_page_pos += chunk_size;
            size -= chunk_size;
        }
        return true;
    }

    bool MemoryOStream::write(const void* data, u64 size)
    {
        if (!size) return true;

        if (m_size + size > m_capacity)
        {
            reserve((m_size + size) << 1);
        }
        memcpy((u8*)m_data + m_size, data, size);
        m_size += size;
        return true;
    }


    bool MemoryOStream::writeString(const char* data)
    {
        write(strlen(data));
        write(data, sizeof(char) * strlen(data));
        return true;
    }


    bool MemoryOStream::writeString(const std::string& data)
    {
        const char* dt = data.c_str();
        write(data.size());
        write(dt, sizeof(char) * data.size());
        return true;
    }


    void MemoryOStream::writeString(StringView string) {
        const i32 size = string.size() + 1;
        write(string.begin, size - 1);
        write((char)0);
    }


    void MemoryOStream::clear()
    {
        m_size = 0;
    }


    void MemoryOStream::free()
    {
        m_allocator->deallocate(m_data);
        m_size = 0;
        m_capacity = 0;
        m_data = nullptr;
    }


    void MemoryOStream::reserve(u64 size)
    {
        if (size <= m_capacity) return;

        ASSERT(m_allocator);
        u8* tmp = (u8*)m_allocator->allocate(size, 1);
        memcpy(tmp, m_data, m_capacity);
        m_allocator->deallocate(m_data);
        m_data = tmp;
        m_capacity = size;
    }


    Span<u8> MemoryOStream::releaseOwnership() {
        Span<u8> res((u8*)m_data, (u8*)m_data + m_capacity);
        m_data = nullptr;
        m_size = m_capacity = 0;
        return res;
    }


    void MemoryOStream::resize(u64 size)
    {
        m_size = size;
        if (size <= m_capacity) return;

        ASSERT(m_allocator);
        u8* tmp = (u8*)m_allocator->allocate(size, 1);
        memcpy(tmp, m_data, m_capacity);
        m_allocator->deallocate(m_data);
        m_data = tmp;
        m_capacity = size;
    }

    MemoryIStream::MemoryIStream(Span<const u8> data)
        : m_data(data.begin())
        , m_size(data.length())
    {}

    MemoryIStream::MemoryIStream(const void* data, u64 size)
        : m_data((const u8*)data)
        , m_size(size)
    {}

    MemoryIStream::MemoryIStream(const MemoryOStream& blob)
        : m_data((const u8*)blob.data())
        , m_size(blob.size())
    {}


    void MemoryIStream::set(const void* data, u64 size) {
        m_data = (u8*)data;
        m_size = size;
        m_pos = 0;
        m_has_overflow = false;
    }


    const void* MemoryIStream::skip(u64 size)
    {
        auto* pos = m_data + m_pos;
        m_pos += size;
        if (m_pos > m_size)
        {
            ASSERT(false);
            m_pos = m_size;
            m_has_overflow = true;
        }

        return (const void*)pos;
    }


    bool MemoryIStream::read(void* data, u64 size)
    {
        if (m_pos + (int)size > m_size)
        {
            for (i32 i = 0; i < size; ++i)
                ((unsigned char*)data)[i] = 0;
            m_has_overflow = true;
            return false;
        }
        if (size)
        {
            memcpy(data, ((char*)m_data) + m_pos, size);
        }
        m_pos += size;
        return true;
    }


    bool MemoryIStream::readString(std::string& str)
    {
        i32 len;
        read(len);
        char* buf = (char*)malloc(len);
        read(buf);
        str = std::string(buf);

        return true;
    }


    bool MemoryIStream::readString(char*& str)
    {
        i32 len;
        read(len);
        char* buf = new char[len];
        read(buf);
        str = buf;

        return true;
    }


    void MemoryOStream::write(const String& string) {
        writeString(string);
    }

    bool MemoryIStream::read(String& string)
    {
        string = readString();
        return true;
    }


    const char* MemoryIStream::readString()
    {
        const char* ret = (const char*)m_data + m_pos;
        while (m_pos < m_size && m_data[m_pos]) ++m_pos;
        // TODO this should be runtime error, not assert
        if (m_pos >= m_size) {
            ASSERT(false);
            m_has_overflow = true;
            return nullptr;
        }
        ++m_pos;
        return ret;
    }
}