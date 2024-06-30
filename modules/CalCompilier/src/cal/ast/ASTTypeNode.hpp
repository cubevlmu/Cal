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
        explicit ASTTypeNode(IAllocator& alloc, const std::string& typeStr);
        explicit ASTTypeNode(IAllocator& alloc, TypeState typeStr);

        virtual ~ASTTypeNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override { return ASTNodeTypes::AST_TYPE; }
        virtual std::string toString() const override; 

        void setType(const std::string& type_Str);
        void setType(TypeState state);
        bool isVerified() const;
        void clear();
        bool isStanderType() const;

        TypeState getTypeState() const { return m_state; }
        std::string getRawTypeStr() const { return m_raw_type; }
        bool isArray() const { return m_is_array; }
        bool isAnyLengthArray() const {
            return m_any_length_arr;
        }
        void setAllowAnyLengthArray(bool val) { m_allow_any_length_arr = val; }

        bool compareType(ASTTypeNode* node);
        bool compareStanderType(ASTTypeNode* node);

        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string m_raw_type;
        bool m_any_length_arr = false;
        bool m_allow_any_length_arr = false;
        
        bool m_is_array = false;
        bool m_is_verified = false;
        Array<int> m_type_params;
        
        TypeState m_state = TypeState::none;
    };
}