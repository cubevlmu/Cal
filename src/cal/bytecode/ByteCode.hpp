#pragma once

#include "OpCodes.hpp"
#include <vector>

namespace cal {

    struct Instruction {
        OpCodes op_code;
        int oprand1;
        int oprand2;
        int oprand3;
    };


    struct Constants {
        enum ConstantTypes {
            Int,
            Double,
            Float,
            Long,
            TEXT
            //TODO More types...
        } const_type;

        union {
            int const_int;
            double const_double;
            float const_float;
            long const_long;
            const char* const_text;
        };
    };


    constexpr int CalBinaryMAGIC = 'CALB';


    struct CalBinaryFile {
        int MAGIC;

        int constants_size;
        std::vector<Constants> consts;

        int bytecode_size;
        std::vector<Instruction> bytecodes;
    };


    class CalBytecode 
    {
    public:
        CalBytecode();

        void write(const std::string& filename);
        void read(const std::string& filename);

        void addConstant(const Constants& constant);
        void addInstruction(const Instruction& instr);

        CalBinaryFile get() const;
        void debugPrint();

    private:
        CalBinaryFile m_file;
    };
}