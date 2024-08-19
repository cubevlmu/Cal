#pragma once

#include "analyzer/ast/NodeType.hpp"
#include "analyzer/ast/expr/ExprNode.hpp"
#include "analyzer/ast/types/NodeType.hpp"
#include "base/allocator/IAllocator.hpp"
#include "base/types/container/HashMap.hpp"
#include "utils/TSingleton.hpp"
#include <string>

namespace cal {

    class ASTNumberNode : public ASTExprNode 
    {
        friend class NumberPool;

        union number_val {
            i8 i8; i16 i16; i32 i32; i64 i64;
            u8 u8; u16 u16; u32 u32; u64 u64;
            float f32; double f64;
        };

        enum number_type {
            unknown = 0,
            I8, I16, I32, I64,
            U8, U16, U32, U64,
            F32, F64,
        };

    private:
        ASTNumberNode(IAllocator& alloc);
        ASTNumberNode(IAllocator& alloc, const number_val& val, number_type type);

        bool parse(const std::string& number_str);
    
    public:
        virtual ASTNodeType* returnType() override;
        virtual ASTTypes nodeType() override { return TYPE_NUMBER; }
        virtual Json::Value buildOutput() override;

        inline bool isVerified() const { return m_isVerified; }
        
        inline i8 getI8() const { return m_numberStorge.i8; }
        inline u8 getU8() const { return m_numberStorge.u8; }
        inline i16 getI16() const { return m_numberStorge.i16; }
        inline u16 getU16() const { return m_numberStorge.u16; }
        inline i32 getI32() const { return m_numberStorge.i32; }
        inline u32 getU32() const { return m_numberStorge.u32; }
        inline i64 getI64() const { return m_numberStorge.i64; }
        inline u64 getU64() const { return m_numberStorge.u64; }
        inline float getF32() const { return m_numberStorge.f32; }
        inline double getF64() const  { return m_numberStorge.f64; }

        template <typename T>
        typename std::enable_if<std::is_arithmetic<T>::value, T>::type get() const;

    private:
        bool m_isVerified;
        ASTNodeType* m_numberType;
        number_val m_numberStorge;
        number_type m_currentPreferedType;
    };


    class NumberPool : public TSingleton<NumberPool> 
    {
    public:
        NumberPool();
        ~NumberPool();

        ASTNumberNode* getNum(const std::string& number_string);
        void clear();
        
        inline ASTNumberNode* getNum(i32 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(u32 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(i64 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(u64 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(i16 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(u16 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(i8 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(u8 val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(float val) { return getNum(std::to_string(val)); }
        inline ASTNumberNode* getNum(double val) { return getNum(std::to_string(val)); }
        
    private:
        HashMap<std::string, ASTNumberNode*> m_pool;
        IAllocator& m_alloc;
    };
}

#include "NumberNode.inl"