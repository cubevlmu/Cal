#pragma once

#include "base/types/Array.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/utils/ASTNodePool.hpp"

namespace cal {

    class ASTTypeNode : public ASTNodeBase, public ASTPoolItem
    {
        friend class ASTTypePool;

    public:
        enum class TypeState {
            i32, i64, i16, i8, f32, boolean, 
            u32, u64, u16, u8, f64, null, defined, none, fvoid
        };

    public:
        explicit ASTTypeNode(IAllocator& alloc);
        explicit ASTTypeNode(IAllocator& alloc, const std::string& typeStr);
        explicit ASTTypeNode(IAllocator& alloc, TypeState typeStr);

        virtual ~ASTTypeNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override { return ASTNodeTypes::AST_TYPE; }
        virtual std::string toString() const override; 
        virtual std::string key() const override { return m_raw_type; };

        void setType(const std::string& type_Str);
        void setType(TypeState state);
        bool isVerified() const;
        void clear();
        bool isStanderType() const;

        TypeState getTypeState() const { return m_state; }
        std::string getOriginTypeStr() const { return m_origin_type_str; }
        std::string getRawTypeStr() const { return m_raw_type; }
        
        bool isArray() const { return m_is_array; }
        bool isAnyLengthArray() const {
            return m_any_length_arr;
        }
        void setAllowAnyLengthArray(bool val) { m_allow_any_length_arr = val; }

        bool compareType(ASTTypeNode* node);
        bool compareStanderType(ASTTypeNode* node);

        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    public:
        static ASTTypeNode* getTypeFromPool(const std::string& raw_type);
        static ASTTypeNode* getTypeFromPool(TypeState state);

    private:
        std::string m_raw_type;
        std::string m_origin_type_str;

        bool m_any_length_arr = false;
        bool m_allow_any_length_arr = false;
        
        bool m_is_array = false;
        bool m_is_verified = false;
        Array<int> m_type_params;
        
        TypeState m_state = TypeState::none;
    };
}