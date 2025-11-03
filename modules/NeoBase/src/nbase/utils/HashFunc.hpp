#pragma once

#include <nbase/base/TypeTraits.hpp>

namespace neo {

    inline u32 getHash(const u8 key)
    {
        return key;
    }

    inline u32 getHash(const i8 key)
    {
        return key;
    }

    inline u32 getHash(const u16 key)
    {
        return key;
    }

    inline u32 getHash(const i16 key)
    {
        return key;
    }

    inline u32 getHash(const i32 key)
    {
        return key;
    }

    inline u32 getHash(const u32 key)
    {
        return key;
    }

    inline u32 getHash(const u64 key)
    {
        return (u32)key + ((u32)(key >> 32) * 23);
    }

    inline u32 getHash(const i64 key)
    {
        return (u32)key + ((u32)(key >> 32) * 23);
    }

    inline u32 getHash(const char key)
    {
        return key;
    }

    inline u32 getHash(const float key)
    {
        return *(u32*)&key;
    }

    inline u32 getHash(const double key)
    {
        return getHash(*(u64*)&key);
    }

    inline u32 getHash(const void* key)
    {
        static const i64 shift = 3;
        return (u32)((i64)(key) >> shift);
    }

    template<typename EnumType>
    inline typename NEnableIf<NIsEnum<EnumType>::Value, u32>::Type getHash(const EnumType key)
    {
        return getHash((__underlying_type(EnumType))key);
    }

    inline void combineHash(u32& hash, const u32 value)
    {
        // Reference: Boost lib
        hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    template<typename T>
    inline void combineHash(u32& hash, const T* value)
    {
        combineHash(hash, getHash(value));
    }

}