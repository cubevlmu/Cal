#pragma once

#include "ASTNodeBase.hpp"
#include "base/types/Array.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include <initializer_list>

namespace cal {

    class ASTBlockNode;

    class ASTFuncArgNode : public ASTNodeBase 
    {
        friend class ASTFuncDefNode;
    public:
        ASTFuncArgNode(IAllocator& alloc);
        virtual ~ASTFuncArgNode() override;

        void set(const std::string& name, ASTTypeNode* type);

        std::string getArgName() const { return m_name; }

        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override {
            return ASTNodeTypes::AST_FUNC_ARG_DEF;
        }
        void registerTo(SyntaxAnalyzer* analyzer);

        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string  m_name;
        ASTTypeNode* m_type;
    };

    
    class ASTFuncDefNode : public ASTNodeBase 
    {
    public:
        enum FuncLinkType {
            STATIC_BODY, DYNAMIC_BODY, NO_BODY
        };

    public:
        ASTFuncDefNode(IAllocator& alloc);
        virtual ~ASTFuncDefNode() override;
        
        virtual Json::Value buildOutput() const override;
        virtual ASTNodeTypes getType() const override
        { return ASTNodeTypes::AST_FUNC_DEF; }

        void setName(const std::string& name);
        std::string getName() const { return m_name; }
        
        void addParams(ASTFuncArgNode* arg);
        void delParams(ASTFuncArgNode* arg);

        void setRetType(ASTTypeNode* type);
        ASTTypeNode* getRetType() const { return m_ret_type; }

        void setLinkType(FuncLinkType type);
        FuncLinkType getLinkType() const { return m_link_state; }

        void setBody(ASTBlockNode* body);
        ASTBlockNode* getBody() const { return m_func_body; }
        bool registerTo(SyntaxAnalyzer* analyzer);
        
        virtual bool checkSyntax(SyntaxAnalyzer* analyzer) const override;

    private:
        std::string m_name;
        Array<ASTFuncArgNode*> m_args;
        ASTTypeNode* m_ret_type;

        ASTBlockNode* m_func_body;
        FuncLinkType m_link_state;
        SyntaxAnalyzer::FuncDetail m_dtl;
    };
}