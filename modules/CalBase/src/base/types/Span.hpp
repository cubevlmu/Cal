#pragma once

#include "globals.hpp"

namespace cal {
    
    template <typename T>
    struct Span
    {
        Span() : m_begin(nullptr), m_end(nullptr) {}
        Span(T* begin, u32 len) : m_begin(begin), m_end(begin + len) {}
        Span(T* begin, T* end) : m_begin(begin), m_end(end) {}
        template <int N> Span(T(&value)[N]) : m_begin(value), m_end(value + N) {}
        T& operator[](u32 idx) const { ASSERT(m_begin + idx < m_end); return m_begin[idx]; }
        operator Span<const T>() const { return Span<const T>(m_begin, m_end); }
        void removePrefix(u32 count) { ASSERT(count <= length()); m_begin += count; }
        void removeSuffix(u32 count) { ASSERT(count <= length()); m_end -= count; }
        [[nodiscard]] Span fromLeft(u32 count) const { ASSERT(count <= length()); return Span(m_begin + count, m_end); }
        [[nodiscard]] Span fromRight(u32 count) const { ASSERT(count <= length()); return Span(m_begin, m_end - count); }
        T& back() { ASSERT(length() > 0); return *(m_end - 1); }
        const T& back() const { ASSERT(length() > 0); return *(m_end - 1); }
        bool equals(const Span<T>& rhs) {
            bool res = true;
            if (length() != rhs.length()) return false;
            for (const T& v : *this) {
                u32 i = u32(&v - m_begin);
                if (v != rhs.m_begin[i]) return false;
            }
            return true;
        }

        template <typename F>
        i32 find(const F& f) const {
            for (u32 i = 0, c = length(); i < c; ++i) {
                if (f(m_begin[i])) return i;
            }
            return -1;
        }

        u32 length() const { return u32(m_end - m_begin); }

        T* begin() const { return m_begin; }
        T* end() const { return m_end; }

        T* m_begin;
        T* m_end;
    };

    template<bool, class T> struct EnableIf {};
    template <class T> struct EnableIf<true, T> { using Type = T; };
    template <class T> inline constexpr bool is_enum_v = __is_enum(T);
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr T operator | (T a, T b) { return T(u64(a) | u64(b)); }
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr T operator & (T a, T b) { return T(u64(a) & u64(b)); }
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr T operator ^ (T a, T b) { return T(u64(a) ^ u64(b)); }
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr void operator |= (T& a, T b) { a = T(u64(a) | u64(b)); }
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr void operator &= (T& a, T b) { a = T(u64(a) & u64(b)); }
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr void operator ^= (T& a, T b) { a = T(u64(a) ^ u64(b)); }
    template <typename T, typename EnableIf<is_enum_v<T>, int>::Type = 0> constexpr T operator ~ (T a) { return T(~u64(a)); }
    template <typename E> bool isFlagSet(E flags, E flag) { return ((u64)flags & (u64)flag); }
    template <typename E> void setFlag(E& flags, E flag, bool set) {
        if (set) flags = E((u64)flags | (u64)flag);
        else flags = E(u64(flags) & ~u64(flag));
    }

}