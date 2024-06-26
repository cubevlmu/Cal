#pragma once

#include "OpCodes.hpp"
#include <unordered_map>

#include "cal/ast/ASTTypeNode.hpp"

namespace cal {

    class CodeGenerator;

    class CalByteCode
    {
        friend class VM;
    public:
        struct Instruction {
            OpCodes code;
            int rand0;
            int rand1;
            int rand2;
        };
        
        struct Constants {
            ASTTypeNode::TypeState constType;
            union {
                i32 i32; u32 u32;
                i64 i64; u64 u64;
                i16 i16; u16 u16;
                i8 i8; u8 u8;
                bool b;
                const char* text;
            };
        };

    public:
        CalByteCode();

        void generate(CodeGenerator* generator);

    private:
        std::vector<Instruction> m_byteCodes;
        std::vector<Constants> m_constants;
    };
}