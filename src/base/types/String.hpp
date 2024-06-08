#pragma once

#include "Hash.hpp"
#include "Span.hpp"
#include <string>

namespace cal {

    struct IAllocator;
    struct Path;
    template <int SIZE> struct StaticString;


    namespace string {
        int stringLength(const char* str);
    }

    struct StringView {
        StringView() {}
        StringView(const char* str) : begin(str), end(str ? str + string::stringLength(str) : 0) {}
        StringView(const char* str, u32 len) : begin(str), end(str + len) {}
        StringView(const char* begin, const char* end) : begin(begin), end(end) {}
        template <int N> StringView(const StaticString<N>& str);

        u32 size() const { return u32(end - begin); }
        char operator[](u32 idx) { ASSERT(!end || begin + idx < end); return begin[idx]; }
        char back() const { ASSERT(end && begin != end); return *(end - 1); }
        void removeSuffix(u32 count) { ASSERT(count <= size()); end -= count; }
        void removePrefix(u32 count) { ASSERT(count <= size()); begin += count; }
        bool empty() const { return begin == end || !begin[0]; }

        std::string toStdString() const {
            return { begin, end };
        }

        const char* toCString() const {
            std::string temp{ begin, end };
            const char* data = temp.c_str();
            return data;
        }

        operator std::string() {
            return toStdString();
        }
        operator const char* () {
            return toCString();
        }

        const char* begin = nullptr;
        const char* end = nullptr;
    };

    namespace string {

        const char* findInsensitive(StringView haystack, StringView needle);
        void toCStringHex(u8 value, Span<char> output);
        void toCStringPretty(i32 value, Span<char> output);
        void toCStringPretty(u32 value, Span<char> output);
        void toCStringPretty(u64 value, Span<char> output);
        char* toCString(bool value, Span<char> output);
        char* toCString(i32 value, Span<char> output);
        char* toCString(i64 value, Span<char> output);
        char* toCString(u64 value, Span<char> output);
        char* toCString(u32 value, Span<char> output);
        char* toCString(float value, Span<char> output, int after_point);
        char* toCString(double value, Span<char> output, int after_point);
        const char* fromCStringOctal(StringView input, u32& value);
        const char* fromCString(StringView input, i32& value);
        const char* fromCString(StringView input, u64& value);
        const char* fromCString(StringView input, i64& value);
        const char* fromCString(StringView input, u32& value);
        const char* fromCString(StringView input, u16& value);
        const char* fromCString(StringView input, bool& value);
        char* copyString(Span<char> output, StringView source);
        char* catString(Span<char> output, StringView source);
        bool makeLowercase(Span<char> output, StringView source);
        bool equalStrings(const char* lhs, const char* rhs);
        bool equalStrings(StringView lhs, StringView rhs);
        bool equalIStrings(StringView lhs, StringView rhs);
        int compareString(StringView lhs, StringView rhs);
        const char* reverseFind(StringView haystack, char c);
        const char* find(StringView str, char needle);
        bool contains(StringView haystack, char needle);
        bool startsWith(StringView str, StringView prefix);
        bool startsWithInsensitive(StringView str, StringView prefix);
        bool endsWith(StringView str, StringView suffix);
        bool endsWithInsensitive(StringView str, StringView suffix);
        bool isLetter(char c);
        bool isNumeric(char c);
        bool isUpperCase(char c);
    }

    // string with included fixed-size storage - i.e. it does not allocate
    // example usage: StaticString<MAX_PATH> path(dir, "/", basename, ".", extension);
    template <int SIZE> struct StaticString {
        StaticString() { data[0] = '\0'; }

        template <typename... Args> StaticString(Args... args) {
            data[0] = '\0';
            append(args...);
        }

        template <typename... Args> void append(Args... args) {
            Span dst(data + string::stringLength(data), data + SIZE);
            int tmp[] = { (add(args, dst), 0)... };
            (void)tmp;
        }

        void operator=(const char* str) { string::copyString(data, str); }
        bool operator<(const char* str) const { return compareString(data, str) < 0; }
        bool operator==(const char* str) const { return string::equalStrings(data, str); }
        bool operator!=(const char* str) const { return !equalStrings(data, str); }

        bool empty() const { return data[0] == '\0'; }
        operator const char* () const { return data; }

        char data[SIZE];

    private:
        template <int S> void add(StaticString<S>& value, Span<char>& dst) { dst.m_begin = string::copyString(dst, value); }
        void add(StringView value, Span<char>& dst) { dst.m_begin = string::copyString(dst, value); }
        void add(StableHash value, Span<char>& dst) { add(value.getHashValue(), dst); }
        void add(float value, Span<char>& dst) { dst.m_begin = string::toCString(value, dst, 3); }
        void add(double value, Span<char>& dst) { dst.m_begin = string::toCString(value, dst, 10); }
        void add(u32 value, Span<char>& dst) { dst.m_begin = string::toCString(value, dst); }
        void add(i32 value, Span<char>& dst) { dst.m_begin = string::toCString(value, dst); }
        void add(u64 value, Span<char>& dst) { dst.m_begin = string::toCString(value, dst); }
        void add(i64 value, Span<char>& dst) { dst.m_begin = string::toCString(value, dst); }

        void add(char value, Span<char>& dst) {
            if (dst.length() < 2) return;
            dst[0] = value;
            dst[1] = '\0';
            dst.m_begin += 1;
        }
    };

    // dynamically allocated string, similar to std::string
    struct String {
        explicit String(IAllocator& allocator);
        String(const String& rhs, u32 start, u32 length);
        String(StringView rhs, IAllocator& allocator);
        String(const String& rhs);
        String(String&& rhs);
        ~String();

        void resize(u32 size);
        char* getMutableData() { return isSmall() ? m_small : m_big; }
        char operator[](u32 index) const;
        void operator=(const String& rhs);
        void operator=(StringView rhs);
        void operator=(String&& rhs);
        void operator+(StringView rhs);
        void operator+(const String& rhs);
        void operator+(String&& rhs);
        bool operator!=(StringView rhs) const;
        bool operator==(StringView rhs) const;
        operator StringView() const { return StringView{ c_str(), m_size }; }
        u32 length() const { return m_size; }
        const char* c_str() const { return isSmall() ? m_small : m_big; }
        template <typename... T>
        void append(T... args) { int tmp[] = { 0, (add(args), 0)... }; }
        void insert(u32 position, const char* value);
        void insert(u32 position, StringView value);
        void eraseAt(u32 position);
        void eraseRange(u32 position, u32 length);

        IAllocator& m_allocator;
    private:
        String& add(StringView value);
        static bool isSmall(u32 size) { return size < sizeof(m_small); }
        bool isSmall() const { return isSmall(m_size); }

        u32 m_size;
        union {
            char* m_big;
            char m_small[16];
        };
    };

    template <int N> StringView::StringView(const StaticString<N>& str)
        : begin(str.data)
        , end(str.data + string::stringLength(str.data))
    {}

}