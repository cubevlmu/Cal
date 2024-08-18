#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/types/String.hpp"
#include "globals.hpp"
#include <sstream>
#include <string>

#define S(...) StringBuilder { __VA_ARGS__ }

namespace cal {

    class StringBuilder
    {
    public:
        StringBuilder();
        StringBuilder(const std::string& str);
        template <typename... T>
        StringBuilder(const T&... args) {
            int tmp[] = { (append(args), 0) ... };
            (void)tmp;
        }

        void append(const std::string& text);
        void append(const char* text);
        void append(i32 i);
        void append(u32 i);
        void append(i64 i);
        void append(u64 i);
        void append(i16 i);
        void append(u16 i);
        void append(i8 i);
        void append(u8 i);
        void append(bool i);
        void append(float i);
        void append(double i);
        void append(const StringBuilder& builder);
        void append(const std::stringstream& ss);

        std::string getString() const {
            return m_ss.str();
        }

        const char* getCString() const {
            std::string str = m_ss.str();
            const char* cstr = str.c_str();
            return cstr;
        }

        String getCalString(IAllocator& alloc) const {
            StringView view(getCString());
            return String(view, alloc);
        }

        template <typename... T>
        void appendAll(const T&... args) {
            int tmp[] = { (append(args), 0) ... };
            (void)tmp;
        }

        operator std::string() const {
            return getString();
        }

        void clear();

    private:
        std::stringstream m_ss;
    };
}