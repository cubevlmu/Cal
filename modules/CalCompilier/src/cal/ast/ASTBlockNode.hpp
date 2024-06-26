#pragma once

#include "ASTNodeBase.hpp"
#include "base/types/Array.hpp"

namespace cal {

    class ASTBlockNode : public ASTNodeBase
    {
    public:
        enum BlockType {
            FUNC_BODY, MODULE_BODY, DEFAULT
        };

    public:
        explicit ASTBlockNode(IAllocator& alloc);
        virtual ~ASTBlockNode() override;

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override {
            return ASTNodeTypes::AST_BLOCK;
        }
        virtual ASTTypeNode* getReturnType() const override;


        bool isEmpty() const { return m_nodes.empty(); }
        void addNode(ASTNodeBase* node);
        void delNode(ASTNodeBase* node);
        void clearBlock();

        void setBlockType(BlockType type) { m_type = type; }
        BlockType getBlockType() const { return m_type; }

        void setFuncReturnType(ASTTypeNode* type);

    private:
        Array<ASTNodeBase*> m_nodes;
        BlockType m_type = BlockType::DEFAULT;
        ASTTypeNode* m_func_ret_type = nullptr;
    };
}