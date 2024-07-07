#include "SyntaxAnalyzer.hpp"

#include "base/Logger.hpp"
#include "base/Utils.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTBlockNode.hpp"
#include "cal/ast/ASTFuncDefNode.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/ast/ASTStructNode.hpp"
#include "system/SysIO.hpp"
#include "system/io/Stream.hpp"
#include "utils/StringBuilder.hpp"

namespace cal {

    SyntaxAnalyzer::SyntaxAnalyzer(IAllocator& alloc)
        : m_alloc(alloc), m_types(alloc)
    {
        m_table = CAL_NEW(m_alloc, SymbolTable)(m_pc, "[anonymous]", m_alloc);
    }


    SyntaxAnalyzer::~SyntaxAnalyzer() {
        CAL_DEL(m_alloc, m_table);
    }


    bool SyntaxAnalyzer::runPass(PreCompile* pc)
    {
        m_pc = pc;
        m_table->setPass(pc);

        if (m_node->getType() != ASTNodeBase::ASTNodeTypes::AST_BLOCK) {
            pc->addError("invalid entry type", this);
            return false;
        }

        ASTBlockNode* rootBlock = (ASTBlockNode*)m_node;
        auto types = rootBlock->pick(ASTBlockNode::SortType::STRUCT_DECLEAR);

        bool cp = true;
        for (ASTNodeBase* node : types) {
            ASTStructNode* nd = (ASTStructNode*)node;
            cp = nd->registerTo(this);
        }
        //TODO

        auto funcs = rootBlock->pick(ASTBlockNode::SortType::FUNC_DECLEAR);

        for (ASTNodeBase* fun : funcs) {
            ASTFuncDefNode* func = (ASTFuncDefNode*)fun;
            cp = func->registerTo(this);
        }
        if (!cp) return cp;


        return m_node->checkSyntax(this);
    }


    void SyntaxAnalyzer::begin(ASTNodeBase* ast)
    {
        m_node = ast;
        ASSERT(m_node);
    }


    void SyntaxAnalyzer::end()
    {
        m_node = nullptr;

    }


    void SyntaxAnalyzer::debugPrint()
    {
        m_table->debugPrint(SymbolTable::DebugPrintOption::FUNCS);
        m_table->debugPrint(SymbolTable::DebugPrintOption::VARIBLES);
        m_table->debugPrint(SymbolTable::DebugPrintOption::STRUCTS);
    }


    bool SyntaxAnalyzer::findTypeByRawType(const std::string& rtp) const
    {
        for (auto* tp : m_types) {
            if (tp->getRawTypeStr() == rtp)
                return true;
        }
        return false;
    }


    bool SyntaxAnalyzer::findTypeByOriginType(const std::string& otp) const
    {
        for (auto* tp : m_types) {
            if (tp->getOriginTypeStr() == otp)
                return true;
        }
        return false;
    }


    SymbolTable::FuncSymbol SymbolTable::INVALID_FUNC { .m_ret_type = nullptr, .name = "INVALID_FUNC" };
    SymbolTable::StructSymbol SymbolTable::INVALID_STRUCT { .name = "INVALID_STRUCT", .selfType = nullptr, .content = nullptr };


    SymbolTable::SymbolTable(PreCompile* pass, const std::string& filename, IAllocator& alloc)
        : m_pass(pass), m_alloc(alloc, "SymbolTable"), m_funcs(m_alloc), m_global_variables(m_alloc), m_structs(m_alloc)
    {
    }


    SymbolTable::~SymbolTable()
    {
        m_funcs.clear();
        m_global_variables.clear();
        m_structs.clear();
    }


    void SymbolTable::setPass(PreCompile* pass)
    {
        m_pass = pass;
    }


    void SymbolTable::addGlobalVariable(const std::string& name, ASTTypeNode* node)
    {
        if (node == nullptr) {
            m_pass->addError("Variable type is invalid", nullptr);
            return;
        }
        if (m_global_variables.find(name).isValid()) {
            m_pass->addError(S("Variable '", name, "' has been declear before this"), nullptr);
            return;
        }
        m_global_variables.insert(name, node);
    }


    void SymbolTable::addFunction(const std::string& name, const FuncSymbol& symbol)
    {
        if (m_funcs.find(name).isValid()) {
            m_pass->addError(S("Function '", name, "' has been declear before this"), nullptr);
            return;
        }
        m_funcs.insert(name, symbol);
    }


    void SymbolTable::addStruct(const std::string& name, const StructSymbol& symbol)
    {
        if (m_structs.find(name).isValid()) {
            m_pass->addError(StringBuilder{ "Struct '", name,"' has been declear before this" }, nullptr);
            return;
        }
        m_structs.insert(name, symbol);
    }


    ASTTypeNode* SymbolTable::getGlobalVariable(const std::string& name) const
    {
        if (!m_global_variables.find(name).isValid()) {
            //m_pass->addError(StringBuilder{ "Variable '", name, "' isn't declear." }, nullptr);
            return nullptr;
        }
        return m_global_variables[name];
    }


    SymbolTable::FuncSymbol SymbolTable::getFunction(const std::string& name)
    {
        if (!m_funcs.find(name).isValid()) {
            //m_pass->addError(StringBuilder{ "Function '", name,"' maybe not been defined" }, nullptr);
            return SymbolTable::INVALID_FUNC;
        }
        return m_funcs[name];
    }


    SymbolTable::StructSymbol SymbolTable::getStruct(const std::string& name)
    {
        if (!m_structs.find(name).isValid()) {
            //m_pass->addError(S("Struct '", name, "' maybe not been decleared."), nullptr);
            return INVALID_STRUCT;
        }
        return m_structs[name];
    }


    bool SymbolTable::removeGlobalVariable(const std::string& name)
    {
        auto* nm = getGlobalVariable(name);
        if (nm == nullptr) {
            return false;
        }
        m_global_variables.eraseIf([&nm](ASTTypeNode* tp) -> bool const { return tp == nm; });
        return !m_global_variables.find(name).isValid();
    }


    bool SymbolTable::removeFunction(const std::string& name)
    {
        auto fn = getFunction(name);
        if (fn == INVALID_FUNC) {
            return false;
        }
        m_funcs.eraseIf([&fn](FuncSymbol& tp) -> bool const { return tp == fn; });
        return !m_funcs.find(name).isValid();
    }


    bool SymbolTable::removeStruct(const std::string& name)
    {
        auto sc = getStruct(name);
        if (sc == INVALID_STRUCT) {
            return false;
        }
        m_structs.eraseIf([&sc](StructSymbol& tp) -> bool const { return tp == sc; });
        return !m_structs.find(name).isValid();
    }


    void SymbolTable::debugPrint(DebugPrintOption opt)
    {
        if (opt == VARIBLES) {
            LogDebug("[SymbolTable] Global variables: ");
            for (auto it = m_global_variables.begin(); it != m_global_variables.end(); ++it) {
                LogDebug("\t VarName: ", it.key(), " VarType: ", it.value()->getRawTypeStr());
            }
        }
        else if (opt == FUNCS) {
            LogDebug("[SymbolTable] Structs");
            for (auto strc : m_structs) {
                LogDebug("\t ", strc.content->toString());
            }
        }
        else if (opt == STRUCTS) {
            LogDebug("[SymbolTable] Functions");
            for (auto func : m_funcs) {
                StringBuilder types {};
                for (auto* tp : func.m_types) {
                    types.appendAll(tp->getOriginTypeStr(), ",");
                }
                std::string typesStr = types.getString();

                LogDebug("\t Name: ", func.name, "(", typesStr.substr(0, typesStr.size() - 1), ") : ", func.m_ret_type->getRawTypeStr());
            }
        }
    }


    void SymbolTable::dumpMap()
    {
        //TODO Not finished

        MemoryOStream stream { m_alloc };
        stream.write(m_funcs.size());
        for(auto func : m_funcs) {
            stream.write(StringView { func.name.c_str() });
            stream.write(StringView { func.m_ret_type->toString().c_str() });

            stream.write(func.m_types.size());
            for (auto tp : func.m_types) {
                stream.writeString(StringView { tp->toString().c_str() });
            }
        }

        // for (auto st : m_structs) {
            
        // }


        //TODO

        platform::OFile f;
        if (!f.open("dump.symtbl")) {
            LogError("[SymbolTable] failed to open dump file");
            return;
        }
        if (!f.write(stream.size())) {
            LogError("[SymbolTable] failed to write dumped data");
            return;
        }
        if (!f.write(stream.data(), sizeof(u8) * stream.size())) {
            LogError("[SymbolTable] failed to write dumped data");
            return;
        }

        f.flush();
        f.close();
        LogDebug("[SymbolTable] dumped");
    }

}