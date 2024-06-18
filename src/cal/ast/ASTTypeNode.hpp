#pragma once

#include "base/types/Array.hpp"
#include "cal/ast/ASTNodeBase.hpp"

namespace cal {

    class ASTTypeNode : public ASTNodeBase 
    {
    public:
        enum class TypeState {
            i32, i64, i16, i8, f32, boolean, 
            u32, u64, u16, u8, f64, null, defined, none
        };

    public:
        explicit ASTTypeNode(IAllocator& alloc);
        virtual ~ASTTypeNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override { return ASTNodeTypes::AST_TYPE; }
        virtual std::string toString() const override; 

        void setType(const std::string& type_Str);
        void setType(TypeState state);
        bool isVerified() const;
        void clear();

        TypeState getTypeState() const { return m_state; }
        std::string getRawTypeStr() const { return m_raw_type; }
        bool isArray() const { return m_is_array; }

        bool compareType(ASTTypeNode* node);

    private:
        std::string m_raw_type;
        
        bool m_is_array, m_is_verified;
        Array<int> m_type_params;
        
        TypeState m_state = TypeState::none;
    };
}