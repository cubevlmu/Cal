#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/types/Array.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "PreCompile.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include <vector>

namespace cal {
    
    class SyntaxAnalyzer : public IPrecompilePass
    {
    public:
        SyntaxAnalyzer(IAllocator& alloc);
        ~SyntaxAnalyzer();

        virtual bool runPass(PreCompile* pc) override;
        virtual void begin(ASTNodeBase* ast) override;
        virtual void end() override;
        virtual void debugPrint() override;

        virtual std::string getName() const override { return "Syntax"; }
    
    public:
        struct FuncDetail {
            std::vector<ASTTypeNode*> m_types {};
            ASTTypeNode* m_ret_type;
            std::string name;
        };

        PreCompile* m_pc;
        Array<ASTTypeNode*> m_types;
        HashMap<std::string, ASTTypeNode*> m_global_variables;
        HashMap<std::string, FuncDetail> m_funcs;

        struct FuncState {
            FuncState(IAllocator& alloc, FuncDetail& dc) : m_local_variables(alloc), func_declear(dc) {
                func_name = dc.name;
            }

            HashMap<std::string, ASTTypeNode*> m_local_variables;
            std::string func_name;
            FuncDetail& func_declear;
            bool isSyntaxWright;
        } *m_now_func_state = nullptr;

        ASTNodeBase* m_node;
        IAllocator& m_alloc;
    };
}