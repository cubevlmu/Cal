#pragma once

#include "ASTNodeBase.hpp"

namespace cal {

    class ASTNumberNode : public ASTNodeBase
    {
    public:
        enum class NumberType {
            non, i32, u32, i64, u64, i8, u8, i16, u16, f32, f64
        };

    public:
        ASTNumberNode(IAllocator& alloc);
        ~ASTNumberNode();

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override
        {
            return ASTNodeTypes::AST_NUMBER;
        }


        void set(const std::string& strNumber);

        template <typename T>
        T getValue() const;
        NumberType getNumberType() const { return m_type; }

    private:
        union {
            float f;
            double d;
            i32 i32;
            u32 u32;
            i64 i64;
            u64 u64;
            i16 i16;
            u16 u16;
            u8 u8;
            i8 i8;
        } val;
        NumberType m_type;
    };


    template<typename T>
    inline T ASTNumberNode::getValue() const {
        if (typeid(T) == typeid(float)) {
            return val.f;
        }
        if (typeid(T) == typeid(double)) {
            return val.d;
        }


        if (typeid(T) == typeid(i32)) {
            return val.i32;
        }
        if (typeid(T) == typeid(u32)) {
            return val.u32;
        }


        if (typeid(T) == typeid(i64)) {
            return val.i64;
        }
        if (typeid(T) == typeid(u64)) {
            return val.u64;
        }


        if (typeid(T) == typeid(i16)) {
            return val.i16;
        }
        if (typeid(T) == typeid(u16)) {
            return val.u16;
        }


        if (typeid(T) == typeid(i8)) {
            return val.i8;
        }
        if (typeid(T) == typeid(u8)) {
            return val.u8;
        }

        return 0;
    }

}