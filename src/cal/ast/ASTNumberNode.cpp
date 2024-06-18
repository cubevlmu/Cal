#include "ASTNumberNode.hpp"
#include "cal/ast/ASTTypeNode.hpp"

#include <json/json.h>

namespace cal {

    ASTNumberNode::ASTNumberNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_type(NumberType::non)
    {
        m_typeNode = CAL_NEW(alloc, ASTTypeNode)(alloc);
    }


    ASTNumberNode::~ASTNumberNode() {
        CAL_DEL(m_allocator, m_typeNode);
    }


    Json::Value ASTNumberNode::buildOutput() const
    {
        Json::Value obj(Json::ValueType::objectValue);

        obj["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        obj["numType"] = (i32)m_type;
        obj["numVal"] = std::to_string(getValue<i64>());

        return obj;
    }


    ASTTypeNode* ASTNumberNode::getReturnType() const {
        if (m_typeNode->isVerified())
            return m_typeNode;
        
        m_typeNode->setType((ASTTypeNode::TypeState)m_type);
        return m_typeNode;
    }


    void ASTNumberNode::set(const std::string& strNumber)
    {
        static const i32 i8max = std::numeric_limits<i8>::max();
        static const i32 i16max = std::numeric_limits<i16>::max();

        // for i32&u32
        try {
            i32 rel = std::stoi(strNumber);

            if (rel < i8max) {
                if (rel >= 0) {
                    val.u8 = (u8)rel;
                    m_type = NumberType::u8;
                }
                else {
                    val.i8 = (i8)rel;
                    m_type = NumberType::i8;
                }

                return;
            }

            if (rel < i16max) {
                if (rel >= 0) {
                    val.u16 = (u16)rel;
                    m_type = NumberType::u16;
                }
                else {
                    val.i16 = (i16)rel;
                    m_type = NumberType::i16;
                }

                return;
            }

            if (rel >= 0) { val.u32 = rel; m_type = NumberType::u32; }
            else { val.i32 = rel; m_type = NumberType::i32; }

            return;
        }
        catch (...) {}

        // for i64&u64
        try {
            i64 rel = std::stoll(strNumber);
            if (rel >= 0) { val.u64 = rel; m_type = NumberType::u64; }
            else { val.i64 = rel; m_type = NumberType::i64; }

            return;
        }
        catch (...) {}

        // for float
        try {
            float vv = std::stof(strNumber);
            val.f = vv;
            m_type = NumberType::f32;

            return;
        }
        catch (...) {}

        // for double
        try {
            double vv = std::stod(strNumber);
            val.d = vv;
            m_type = NumberType::f64;

            return;
        }
        catch (...) {}
    }

}