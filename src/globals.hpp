#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <csignal>


#define DEBUGING

namespace cal {

// For Enable CVK's Debug Layer
    #define CVK_DEBUG
// 

    template <typename T>
    using ref = std::shared_ptr<T>;
    template <typename T>
    using unique = std::unique_ptr<T>;

    template <typename T, typename... Args>
    constexpr ref<T> create_ref(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
    template <typename T, typename... Args>
    constexpr unique<T> create_unique(Args &&...args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    using i8 = char;
    using u8 = unsigned char;
    using i16 = short;
    using u16 = unsigned short;
    using i32 = int;
    using u32 = unsigned int;
#ifdef cb_winnt
    using i64 = long long;
    using u64 = unsigned long long;
#else
    using i64 = long;
    using u64 = unsigned long;
#endif
    using uintptr = u64;

    typedef unsigned char cbyte;

    template <typename T, u32 count> constexpr u32 lengthOf(const T(&)[count])
    {
        return count;
    };

#ifdef cb_winnt
#define cb_debug_break() __debugbreak()
#define cb_inline __forceinline
#define cb_restrict __restrict
#else
#define cb_debug_break() throw std::runtime_error("failed to assert") 
//raise(SIGTRAP)
#define cb_inline __attribute__((always_inline)) inline
#define cb_restrict __restrict__
#endif

#define ASSERT(x)                                  \
    do                                             \
    {                                              \
        const volatile bool _assert_result = !(x); \
        if (_assert_result)                        \
            cb_debug_break();                      \
    } while (false) 



#ifdef cb_debug


#define CB_ASSERT(x)          \
    {                         \
        if (!(x))             \
        {                     \
            CError("Failed"); \
            throw "awa";      \
        }                     \
    }

#define CB_CORE_ASSERT(x, ...)                \
    {                                         \
        if (!(x))                             \
        {                                     \
            CError("Faild {0}", __VA_ARGS__); \
            throw __VA_ARGS__;                \
        }                                     \
    }

#else
#define CB_ASSERT(...)
#define CB_CORE_ASSERT(...)
#endif


}