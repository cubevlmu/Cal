#pragma once

#include "globals.hpp"

namespace cal {

    struct RuntimeHash {
        static RuntimeHash fromU64(u64 hash);
        RuntimeHash() {}
        explicit RuntimeHash(const char* string);
        RuntimeHash(const void* data, u32 len);

        bool operator != (RuntimeHash rhs) const { return hash != rhs.hash; }
        bool operator == (RuntimeHash rhs) const { return hash == rhs.hash; }
        operator u64() const { return hash; }

        u64 getHashValue() const { return hash; }
    private:
        u64 hash = 0;
#ifdef TOY_DEBUG
        const char* debug_str = nullptr;
#endif
    };


    struct RuntimeHash32 {
        static RuntimeHash32 fromU32(u32 hash);
        RuntimeHash32() {}
        explicit RuntimeHash32(const char* string);
        RuntimeHash32(const void* data, u32 len);

        bool operator != (RuntimeHash32 rhs) const { return hash != rhs.hash; }
        bool operator == (RuntimeHash32 rhs) const { return hash == rhs.hash; }
        operator u32() const { return getHashValue(); }

        u32 getHashValue() const { return hash; }
    private:
        u32 hash = 0;
    };


    // use if you want to serialize it
    // 64bits 
    struct StableHash {
        static StableHash fromU64(u64 hash);
        StableHash() {}
        explicit StableHash(const char* str);
        StableHash(const void* data, u32 len);

        bool operator != (StableHash rhs) const { return hash != rhs.hash; }
        bool operator == (StableHash rhs) const { return hash == rhs.hash; }
        bool operator < (StableHash rhs) const { return hash < rhs.hash; }
        operator u64() const { return hash; }

        u64 getHashValue() const { return hash; }
    private:
        u64 hash = 0;
    };


    struct StableHash32 {
        static StableHash32 fromU32(u32 hash);
        StableHash32() {}
        explicit StableHash32(const char* string);
        StableHash32(const void* data, u32 len);

        bool operator != (StableHash32 rhs) const { return hash != rhs.hash; }
        bool operator == (StableHash32 rhs) const { return hash == rhs.hash; }
        bool operator < (StableHash32 rhs) const { return hash < rhs.hash; }
        operator u32() const { return hash; }

        u32 getHashValue() const { return hash; }
    private:
        u32 hash = 0;
    };

    using FilePathHash = StableHash;
    using BoneNameHash = StableHash;

    struct RollingStableHasher {
        void begin();
        void update(const void* data, u32 len);
        StableHash32 end();
        StableHash end64();
    };

    struct RollingHasher {
        void begin();
        void update(const void* data, u32 len);
        RuntimeHash32 end();
    };

    template <typename Key> struct HashFunc;

    template<> struct HashFunc<RuntimeHash> {
        static u32 get(const RuntimeHash& k) {
            const u64 hash = k.getHashValue();
            return u32(hash ^ (hash >> 16));
        }
    };

    template<> struct HashFunc<StableHash> {
        static u32 get(const StableHash& k) {
            const u64 hash = k.getHashValue();
            return u32(hash ^ (hash >> 16));
        }
    };

    template<> struct HashFunc<StableHash32> {
        static u32 get(const StableHash32& k) {
            return k.getHashValue();
        }
    };

    template<> struct HashFunc<RuntimeHash32> {
        static u32 get(const RuntimeHash32& k) {
            return k.getHashValue();
        }
    };
}