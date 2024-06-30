#pragma once

#include <memory>
#ifndef _WIN32
#include <signal.h> // SIGTRAP
#endif

#include "base/Lib.hpp"
#include <cstring>

namespace cal {


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

#if !defined(_WIN32) && !defined(__linux__) && !defined (__MACH__)
#error Platform not supported
#endif

#ifndef ASSERT
#ifdef CAL_DEBUG
#ifdef _WIN32
#define CAL_DEBUG_BREAK() __debugbreak()
#else
#define CAL_DEBUG_BREAK()  raise(SIGTRAP) 
#endif
#define ASSERT(x) do { const bool CAL_assert_b____ = !(x); if(CAL_assert_b____) CAL_DEBUG_BREAK(); } while (false)
#else
#if defined _MSC_VER && !defined __clang__
#define ASSERT(x) __assume(x)
#else
#define ASSERT(x) { false ? (void)(x) : (void)0; }
#endif
#endif
#endif


#ifdef _WIN32
#define CAL_LIBRARY_EXPORT __declspec(dllexport)
#define CAL_LIBRARY_IMPORT __declspec(dllimport)
#define CAL_FORCE_INLINE __forceinline
#define CAL_RESTRICT __restrict
#else 
#define CAL_LIBRARY_EXPORT __attribute__((visibility("default")))
#define CAL_LIBRARY_IMPORT 
#define CAL_FORCE_INLINE __attribute__((always_inline)) inline
#define CAL_RESTRICT __restrict__
#endif

#ifdef STATIC_PLUGINS
#define CAL_API
#elif defined BUILDING
#define CAL_API CAL_LIBRARY_EXPORT
#else
#define CAL_API CAL_LIBRARY_IMPORT
#endif

#ifdef _MSC_VER
#pragma warning(error : 4101)
#pragma warning(error : 4127)
#pragma warning(error : 4263)
#pragma warning(error : 4265)
#pragma warning(error : 4296)
#pragma warning(error : 4456)
#pragma warning(error : 4062)
#pragma warning(error : 5233)
#pragma warning(error : 5245)
#pragma warning(disable : 4251)
    // this is disabled because VS19 16.5.0 has false positives :(
#pragma warning(disable : 4724)
#if _MSC_VER == 1900 
#pragma warning(disable : 4091)
#endif
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wignored-pragma-optimize"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wchar-subscripts"
#endif

}

namespace cal {

#ifdef MAX_PATH
#undef MAX_PATH
#endif

    enum { MAX_PATH = 260 };

    using i8 = char;
    using u8 = unsigned char;
    using i16 = short;
    using u16 = unsigned short;
    using i32 = int;
    using u32 = unsigned int;

#ifdef _WIN32
    using i64 = long long;
    using u64 = unsigned long long;
#else	
    using i64 = long;
    using u64 = unsigned long;
#endif
    using uintptr = u64;
    using ptr = size_t;

    using fi32 = int_fast32_t; ///< Fast signed integer at least 32bit
    using fu32 = uint_fast32_t; ///< Fast unsigned integer at least 32bit
    using Second = double;

    constexpr i8 kMaxI8 = std::numeric_limits<i8>::max();
    constexpr i8 kMinI8 = std::numeric_limits<i8>::min();

    constexpr i16 kMaxI16 = std::numeric_limits<i16>::max();
    constexpr i16 kMinI16 = std::numeric_limits<i16>::min();

    constexpr i32 kMaxI32 = std::numeric_limits<i32>::max();
    constexpr i32 kMinI32 = std::numeric_limits<i32>::min();

    constexpr i64 kMaxi64 = std::numeric_limits<i64>::max();
    constexpr i64 kMini64 = std::numeric_limits<i64>::min();

    constexpr fi32 kMaxI = std::numeric_limits<fi32>::max();
    constexpr fi32 kMinI = std::numeric_limits<fi32>::min();

    constexpr u8 kMaxU8 = std::numeric_limits<u8>::max();
    constexpr u8 kMinU8 = std::numeric_limits<u8>::min();

    constexpr u16 kMaxU16 = std::numeric_limits<u16>::max();
    constexpr u16 kMinU16 = std::numeric_limits<u16>::min();

    constexpr u32 kMaxU32 = std::numeric_limits<u32>::max();
    constexpr u32 kMinU32 = std::numeric_limits<u32>::min();

    constexpr u64 kMaxU64 = std::numeric_limits<u64>::max();
    constexpr u64 kMinU64 = std::numeric_limits<u64>::min();

    constexpr fu32 kMaxU = std::numeric_limits<fu32>::max();
    constexpr fu32 kMinU = std::numeric_limits<fu32>::min();

    constexpr ptr kMaxPtrSize = std::numeric_limits<ptr>::max();
    constexpr ptr kMinPtrSize = std::numeric_limits<ptr>::min();

    constexpr float kMaxF32 = std::numeric_limits<float>::max();
    constexpr float kMinF32 = -std::numeric_limits<float>::max();

    constexpr double kMaxF64 = std::numeric_limits<double>::max();
    constexpr double kMinF64 = -std::numeric_limits<double>::max();


    static_assert(sizeof(uintptr) == sizeof(void*), "Incorrect size of uintptr");
    static_assert(sizeof(i64) == 8, "Incorrect size of i64");
    static_assert(sizeof(i32) == 4, "Incorrect size of i32");
    static_assert(sizeof(i16) == 2, "Incorrect size of i16");
    static_assert(sizeof(i8) == 1, "Incorrect size of i8");
    static_assert(sizeof(ptr) == sizeof(void*), "Incorrect size for size_t");

    template <typename T, u32 count> constexpr u32 lengthOf(const T(&)[count])
    {
        return count;
    };

    /// A simple template trick to remove the pointer from one type
///
/// Example:
/// @code
/// using Ptr = double*;
/// RemovePointer<Ptr>::Type b = 666.0;
/// @endcode
/// The b is of type double
    template<typename T>
    struct RemovePointer;

    template<typename T>
    struct RemovePointer<T*>
    {
        using Type = T;
    };

    template<typename T>
    struct RemovePointer<const T*>
    {
        using Type = T;
    };


    class [[nodiscard]] Error
    {
    public:
        static constexpr i32 kNone = 0;
        static constexpr i32 kOutOfMemory = 1;
        static constexpr i32 kFunctionFailed = 2; ///< External operation failed
        static constexpr i32 kUserData = 3;

        static constexpr i32 kFileNotFound = 4;
        static constexpr i32 kFileAccess = 5; ///< Read/write access error

        static constexpr i32 kUnknown = 6;

        /// Construct using an error code.
        Error(i32 code)
            : m_code(code)
        { }

        /// Copy.
        Error(const Error& b)
            : m_code(b.m_code)
        { }

        /// Copy.
        Error& operator=(const Error& b)
        {
            m_code = b.m_code;
            return *this;
        }

        /// Compare.
        bool operator==(const Error& b) const
        {
            return m_code == b.m_code;
        }

        /// Compare.
        bool operator==(i32 code) const
        {
            return m_code == code;
        }

        /// Compare.
        bool operator!=(const Error& b) const
        {
            return m_code != b.m_code;
        }

        /// Compare.
        bool operator!=(i32 code) const
        {
            return m_code != code;
        }

        /// Check if it is an error.
        explicit operator bool() const
        {
            if (m_code != kNone) [[unlikely]]
                {
                    return true;
                }
            else
            {
                return false;
            }
        }

        i32 _getCode() const
        {
            return m_code;
        }
    private:
        i32 m_code = kNone;
    };

} // namespace cal
