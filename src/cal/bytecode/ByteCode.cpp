#include "ByteCode.hpp"

#include "base/Logger.hpp"

#include <ostream>
#include <fstream>

namespace cal {


    static std::string parseToString(const Constants& vv) {
        switch (vv.const_type)
        {
        case Constants::Int:
            return std::to_string(vv.const_int);
            break;
        case Constants::Double:
            return std::to_string(vv.const_double);
            break;
        case Constants::Float:
            return std::to_string(vv.const_float);
            break;
        case Constants::Long:
            return std::to_string(vv.const_long);
            break;
        case Constants::TEXT:
            return std::string(vv.const_text);
            break;
        }
    }


    const char* ConstantTypes[] = {
        "Int",
        "Double",
        "Float",
        "Long",
        "TEXT"
        //TODO More types...
    };


    const char* OpCodesNames[] = {
        "OP_LOAD_VAR",
        "OP_LOAD_CONST",
        "OP_ADD",
        "OP_SUB",
        "OP_MUL",
        "OP_DIV", 
        "OP_STORE" 
    };


    CalBytecode::CalBytecode() : m_file()
    {
    }


    void CalBytecode::write(const std::string& filename)
    {
        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            LogError("[CalBytecode] Cannot open file");
            ASSERT(false);
        }

        out.write(reinterpret_cast<char*>(&m_file.MAGIC), sizeof(m_file.MAGIC));
        out.write(reinterpret_cast<char*>(&m_file.constants_size), sizeof(m_file.constants_size));
        for (const auto& constant : m_file.consts) {
            out.write(reinterpret_cast<const char*>(&constant.const_type), sizeof(constant.const_type));
            switch (constant.const_type) {
            case Constants::Int:
                out.write(reinterpret_cast<const char*>(&constant.const_int), sizeof(constant.const_int));
                break;
            case Constants::Double:
                out.write(reinterpret_cast<const char*>(&constant.const_double), sizeof(constant.const_double));
                break;
            case Constants::Float:
                out.write(reinterpret_cast<const char*>(&constant.const_float), sizeof(constant.const_float));
                break;
            case Constants::Long:
                out.write(reinterpret_cast<const char*>(&constant.const_long), sizeof(constant.const_long));
                break;
            case Constants::TEXT:
                out.write(constant.const_text, strlen(constant.const_text) + 1);
                break;
            }
        }
        out.write(reinterpret_cast<char*>(&m_file.bytecode_size), sizeof(m_file.bytecode_size));
        for (const auto& instr : m_file.bytecodes) {
            out.write(reinterpret_cast<const char*>(&instr), sizeof(instr));
        }
    }


    void CalBytecode::read(const std::string& filename)
    {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            LogError("[CalBytecode] Cannot open file");
            ASSERT(false);
        }

        in.read(reinterpret_cast<char*>(&m_file.MAGIC), sizeof(m_file.MAGIC));
        in.read(reinterpret_cast<char*>(&m_file.constants_size), sizeof(m_file.constants_size));
        m_file.consts.resize(m_file.constants_size);
        for (auto& constant : m_file.consts) {
            in.read(reinterpret_cast<char*>(&constant.const_type), sizeof(constant.const_type));
            switch (constant.const_type) {
            case Constants::Int:
                in.read(reinterpret_cast<char*>(&constant.const_int), sizeof(constant.const_int));
                break;
            case Constants::Double:
                in.read(reinterpret_cast<char*>(&constant.const_double), sizeof(constant.const_double));
                break;
            case Constants::Float:
                in.read(reinterpret_cast<char*>(&constant.const_float), sizeof(constant.const_float));
                break;
            case Constants::Long:
                in.read(reinterpret_cast<char*>(&constant.const_long), sizeof(constant.const_long));
                break;
            case Constants::TEXT: {
                std::string text;
                std::getline(in, text, '\0');
                constant.const_text = strdup(text.c_str());
                break;
            }
            }
        }

        in.read(reinterpret_cast<char*>(&m_file.bytecode_size), sizeof(m_file.bytecode_size));
        m_file.bytecodes.resize(m_file.bytecode_size);
        for (auto& instr : m_file.bytecodes) {
            in.read(reinterpret_cast<char*>(&instr), sizeof(instr));
        }
    }


    void CalBytecode::addConstant(const Constants& constant)
    {
        m_file.consts.push_back(constant);
        m_file.constants_size++;
    }


    void CalBytecode::addInstruction(const Instruction& instr)
    {
        m_file.bytecodes.push_back(instr);
        m_file.bytecode_size++;
    }


    CalBinaryFile CalBytecode::get() const
    {
        return m_file;
    }


    void CalBytecode::debugPrint()
    {
        LogDebug("[CalByteCode] \n\t[Constants] \n");
        for (const auto& consts : this->m_file.consts) {
            LogDebug("\t Type ", ConstantTypes[consts.const_type], " Value ", parseToString(consts));
        }
        LogDebug("\n\t[Instructions] \n");
        for(const auto& ins : this->m_file.bytecodes) {
            LogDebug("\t ", OpCodesNames[ins.op_code], " ", ins.oprand1, " ", ins.oprand2, " ", ins.oprand3);
        }
    }


}