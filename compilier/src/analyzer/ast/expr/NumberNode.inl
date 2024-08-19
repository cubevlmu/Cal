#pragma once

#include "NumberNode.hpp"

namespace cal {
    template<typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value, T>::type ASTNumberNode::get() const
    {
        if(typeid(T) == typeid(i32)) {
            return getI32();
        }
        if(typeid(T) == typeid(u32)) {
            return getU32();
        }
        if(typeid(T) == typeid(i64)) {
            return getI64();
        }
        if(typeid(T) == typeid(u64)) {
            return getU64();
        }
        if(typeid(T) == typeid(i16)) {
            return getI16();
        }
        if(typeid(T) == typeid(u16)) {
            return getU16();
        }
        if(typeid(T) == typeid(i8)) {
            return getI8();
        }
        if(typeid(T) == typeid(u8)) {
            return getU8();
        }
        if(typeid(T) == typeid(float)) {
            return getF32();
        }
        if(typeid(T) == typeid(double)) {
            return getF64();
        }

        return 0;
    }
}