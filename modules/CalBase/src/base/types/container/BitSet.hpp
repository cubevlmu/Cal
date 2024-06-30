#pragma once

#include "globals.hpp"
#include "base/types/Array.hpp"

namespace cal {

    /// Easy bit manipulation.
    /// @tparam kBitCount The number of bits.
    /// @tparam TChunkType The type of the chunks that the bitset consists. By default it's U8.
    template<u32 kBitCount, typename TChunkType = u8>
    class BitSet
    {
    private:
        using ChunkType = TChunkType;

        /// Number of bits a chunk holds.
        static constexpr u32 kChunkBitCount = sizeof(ChunkType) * 8;

        /// Number of chunks.
        static constexpr u32 kChunkCount = (kBitCount + (kChunkBitCount - 1)) / kChunkBitCount;

    public:
        /// Constructor. It will set all the bits or unset them.
        BitSet(bool set, IAllocator& alloc)
            : m_chunks(alloc),
              m_alloc(alloc)
        {
            ASSERT(set == 0 || set == 1);
            if (!set)
                unsetAll();
            else
                setAll();
        }

        /// Copy.
        BitSet(const BitSet& b)
            : m_chunks(b.m_chunks),
              m_alloc(b.m_alloc)
        {
        }

        /// Copy.
        BitSet& operator=(const BitSet& b)
        {
            m_chunks = b.m_chunks;
            return *this;
        }

        /// Bitwise or between this and @a b sets.
        BitSet operator|(const BitSet& b) const
        {
            BitSet out;
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                out.m_chunks[i] = m_chunks[i] | b.m_chunks[i];
            }
            return out;
        }

        /// Bitwise or between this and @a b sets.
        BitSet& operator|=(const BitSet& b)
        {
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                m_chunks[i] = m_chunks[i] | b.m_chunks[i];
            }
            return *this;
        }

        /// Bitwise and between this and @a b sets.
        BitSet operator&(const BitSet& b) const
        {
            BitSet out;
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                out.m_chunks[i] = m_chunks[i] & b.m_chunks[i];
            }
            return out;
        }

        /// Bitwise and between this and @a b sets.
        BitSet& operator&=(const BitSet& b)
        {
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                m_chunks[i] = m_chunks[i] & b.m_chunks[i];
            }
            return *this;
        }

        /// Bitwise xor between this and @a b sets.
        BitSet operator^(const BitSet& b) const
        {
            BitSet out;
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                out.m_chunks[i] = m_chunks[i] ^ b.m_chunks[i];
            }
            return out;
        }

        /// Bitwise xor between this and @a b sets.
        BitSet& operator^=(const BitSet& b)
        {
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                m_chunks[i] = m_chunks[i] ^ b.m_chunks[i];
            }
            return *this;
        }

        /// Bitwise not of self.
        BitSet operator~() const
        {
            BitSet out;
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                out.m_chunks[i] = TChunkType(~m_chunks[i]);
            }
            out.zeroUnusedBits();
            return out;
        }

        bool operator==(const BitSet& b) const
        {
            bool same = m_chunks[0] == b.m_chunks[0];
            for (u32 i = 1; i < kChunkCount; ++i)
            {
                same = same && (m_chunks[i] == b.m_chunks[i]);
            }
            return same;
        }

        bool operator!=(const BitSet& b) const
        {
            return !(*this == b);
        }

        bool operator!() const
        {
            return !getAnySet();
        }

        explicit operator bool() const
        {
            return getAnySet();
        }

        /// Set or unset a bit at the given position.
        template<typename TInt>
        BitSet& set(TInt pos, bool setBit = true)
        {
            u32 high, low;
            position(u32(pos), high, low);
            const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
            m_chunks[high] = (setBit) ? ChunkType(m_chunks[high] | mask) : ChunkType(m_chunks[high] & ~mask);
            return *this;
        }

        /// Set multiple bits.
        template<typename TInt>
        BitSet& set(std::initializer_list<TInt> list, bool setBits = true)
        {
            for (auto it : list)
            {
                set(it, setBits);
            }
            return *this;
        }

        /// Set all bits.
        BitSet& setAll()
        {
            memset(&m_chunks[0], 0xFF, sizeof(m_chunks));
            zeroUnusedBits();
            return *this;
        }

        /// Unset a bit (set to zero) at the given position.
        template<typename TInt>
        BitSet& unset(TInt pos)
        {
            return set(pos, false);
        }

        /// Unset multiple bits.
        template<typename TInt>
        BitSet& unset(std::initializer_list<TInt> list)
        {
            return set(list, false);
        }

        /// Unset all bits.
        BitSet& unsetAll()
        {
            memset(&m_chunks[0], 0, sizeof(m_chunks));
            return *this;
        }

        /// Flip the bits at the given position. It will go from 1 to 0 or from 0 to 1.
        template<typename TInt>
        BitSet& flip(TInt pos)
        {
            u32 high, low;
            position(u32(pos), high, low);
            const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
            m_chunks[high] ^= mask;
            return *this;
        }

        /// Return true if the bit is set or false if it's not.
        template<typename TInt>
        bool get(TInt pos) const
        {
            u32 high, low;
            position(u32(pos), high, low);
            const ChunkType mask = ChunkType(ChunkType(1) << ChunkType(low));
            return (m_chunks[high] & mask) != 0;
        }

        /// Any are enabled.
        bool getAnySet() const
        {
            const BitSet kZero(false, m_alloc);
            return *this != kZero;
        }

        /// Count bits.
        u32 getSetBitCount() const
        {
            u32 count = 0;
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                count += __builtin_popcountl(m_chunks[i]);
            }
            return count;
        }

        /// Get the most significant bit that is enabled. Or kMaxu32 if all is zero.
        u32 getMostSignificantBit() const
        {
            u32 i = kChunkCount;
            while (i--)
            {
                const u64 bits = m_chunks[i];
                if (bits != 0)
                {
                    const u32 msb = u32(__builtin_clzll(bits));
                    return (63 - msb) + (i * kChunkBitCount);
                }
            }

            return kMaxU32;
        }

        /// Get the least significant bit that is enabled. Or kMaxu32 if all is zero.
        u32 getLeastSignificantBit() const
        {
            for (u32 i = 0; i < kChunkCount; ++i)
            {
                const u64 bits = m_chunks[i];
                if (bits != 0)
                {
                    const u32 lsb = u32(__builtin_ctzll(bits));
                    return lsb + (i * kChunkBitCount);
                }
            }

            return kMaxU32;
        }

        Array<TChunkType> getData() const
        {
            return m_chunks;
        }

        /// Unset the N least significant bits of the bitset.
        BitSet& unsetNLeastSignificantBits(u32 n)
        {
            ASSERT(n);
            u32 high, low;
            position(n - 1, high, low);
            for (u32 i = 0; i < high; ++i)
            {
                m_chunks[i] = 0;
            }

            // This could be a simple 1<<(low+1) but that may overflow so...
            ChunkType mask = (ChunkType(1) << ChunkType(low)) - ChunkType(1);
            mask <<= 1;
            mask |= 1;

            mask = ~mask;
            m_chunks[high] &= mask;

            return *this;
        }

    private:
        IAllocator& m_alloc;
        Array<ChunkType> m_chunks;

        BitSet()
        {
        }

        static void position(u32 bit, u32& high, u32& low)
        {
            ASSERT(bit < kBitCount);
            high = bit / kChunkBitCount;
            low = bit % kChunkBitCount;
            ASSERT(high < kChunkCount);
            ASSERT(low < kChunkBitCount);
        }

        /// Zero the unused bits.
        void zeroUnusedBits()
        {
            constexpr ChunkType kUnusedBits = kChunkCount * kChunkBitCount - kBitCount;
            constexpr ChunkType kUsedBitmask = std::numeric_limits<ChunkType>::max() >> kUnusedBits;
            if (kUsedBitmask > 0)
            {
                m_chunks[kChunkCount - 1] &= kUsedBitmask;
            }
        }
    };
}