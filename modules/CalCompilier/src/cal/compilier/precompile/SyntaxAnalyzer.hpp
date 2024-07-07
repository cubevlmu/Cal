#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/allocator/TagAllocator.hpp"
#include "base/types/Array.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "PreCompile.hpp"
#include "cal/ast/ASTStructNode.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include <string>
#include <vector>

namespace cal {

    class SymbolTable
    {
    public:
        struct FuncSymbol {
            std::vector<ASTTypeNode*> m_types{};
            ASTTypeNode* m_ret_type;
            std::string name;

            bool operator ==(const FuncSymbol& obj) {
                return obj.m_types == m_types && obj.name == name && obj.m_ret_type == m_ret_type;
            }
            bool operator !=(const FuncSymbol& obj) {
                return obj.m_types != m_types && obj.name != name && obj.m_ret_type != m_ret_type;
            }
        };
        static FuncSymbol INVALID_FUNC;


        struct StructSymbol {
            std::string name;
            ASTTypeNode* selfType;
            ASTStructNode* content;

            bool operator ==(const StructSymbol& obj) {
                return obj.name == name && obj.selfType == selfType && obj.content == content;
            }
            bool operator !=(const StructSymbol& obj) {
                return obj.name != name && obj.selfType != selfType && obj.content != content;
            }
        };
        static StructSymbol INVALID_STRUCT;

        enum DebugPrintOption {
            VARIBLES, STRUCTS, FUNCS
        };

    public:
        SymbolTable(PreCompile* pass, const std::string& filename, IAllocator& alloc);
        ~SymbolTable();

        void setPass(PreCompile* pass);

        void addGlobalVariable(const std::string& name, ASTTypeNode* node);
        void addFunction(const std::string& name, const FuncSymbol& symbol);
        void addStruct(const std::string& name, const StructSymbol& symbol);

        ASTTypeNode* getGlobalVariable(const std::string& name) const;
        FuncSymbol getFunction(const std::string& name);
        StructSymbol getStruct(const std::string& name);

        bool removeGlobalVariable(const std::string& name);
        bool removeFunction(const std::string& name);
        bool removeStruct(const std::string& name);

        void debugPrint(DebugPrintOption opt);
        void dumpMap();

    private:
        PreCompile* m_pass;
        TagAllocator m_alloc;

        HashMap<std::string, ASTTypeNode*> m_global_variables;
        HashMap<std::string, FuncSymbol> m_funcs;
        HashMap<std::string, StructSymbol> m_structs;
    };


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
        inline IAllocator& getAllocator() { return m_alloc; }

        bool findTypeByRawType(const std::string& rtp) const;
        bool findTypeByOriginType(const std::string& otp) const;
        inline bool findTypeByTypeObj(ASTTypeNode* node) {
            i32 idx = m_types.indexOf(node);
            return idx != -1;
        }
        inline void insertType(ASTTypeNode* node) {
            m_types.push(node);
            m_types.removeDuplicates();
        }

    public:
        SymbolTable* m_table;
        PreCompile* m_pc;
        ASTNodeBase* m_node;

        struct FuncState {
            FuncState(IAllocator& alloc, SymbolTable::FuncSymbol& dc) : m_local_variables(alloc), func_declear(dc) {
                func_name = dc.name;
            }

            HashMap<std::string, ASTTypeNode*> m_local_variables;
            std::string func_name;
            SymbolTable::FuncSymbol& func_declear;
            bool isSyntaxWright;
        } *m_now_func_state = nullptr;

    private:
        Array<ASTTypeNode*> m_types;
        IAllocator& m_alloc;

    };
}