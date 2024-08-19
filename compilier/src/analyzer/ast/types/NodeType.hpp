#pragma once

#include "analyzer/ast/NodeBase.hpp"
#include "analyzer/ast/NodeType.hpp"
#include "base/allocator/IAllocator.hpp"
#include "base/types/Array.hpp"
#include "globals.hpp"
#include <string>

namespace cal {

    class TypePool;

    class ASTNodeType : public ASTNodeBase
    {
        friend class TypePool;
        friend class ASTNumberNode;

        static const char* TYPE_STRS[];
        static int checkIdx(const std::string& type);

    public:
        enum Types : i8 {
            unknown = 0,
            i8, i16, i32, i64,
            u8, u16, u32, u64,
            f32, f64,
            boolean,
            custom
        };

    private:
        ASTNodeType(IAllocator& alloc, const std::string& type);
        ASTNodeType(IAllocator& alloc, Types type);

        bool parse(const std::string& raw);
        inline static const char* typeToStr(Types type) {
            return TYPE_STRS[(int)type];
        }

    public:
        virtual ASTTypes nodeType() override { return ASTTypes::TYPE_NODE; }
        virtual Json::Value buildOutput() override;
        virtual std::string toString() override;

        bool isArray() { return m_isArray; }
        bool isStanderType();
        bool isCustomType();
        bool isParsedSucceed() { return m_isVerified; }

        std::string getRawTypeName() { return m_raw_type_str; }
        std::string getTypeName() { return m_type_str; }
        Types getType() { return m_type; }

        virtual bool compareType(ASTNodeType* type);

    private:
        std::string m_raw_type_str;
        std::string m_type_str;

        ASTNodeType* m_template_type = nullptr;

        Types m_type;
        bool m_isArray;
        bool m_isTemplate;
        bool m_isVerified;

        bool m_array_ref;
        int m_array_dimension;
        struct array_length_parm {
            union {
                const char* c;
                uint32_t i;
                uint64_t l;
            };
            enum {
                RefName, I32, I64
            } type;
        };
        Array<array_length_parm> m_array_length_parms;
    };
}