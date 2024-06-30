#include "SyntaxAnalyzer.hpp"

#include "base/Logger.hpp"
#include "base/Utils.hpp"
#include "base/types/container/HashMap.hpp"
#include "cal/ast/ASTBlockNode.hpp"
#include "cal/ast/ASTFuncDefNode.hpp"
#include "cal/ast/ASTNodeBase.hpp"

namespace cal {

    SyntaxAnalyzer::SyntaxAnalyzer(IAllocator& alloc)
        : m_alloc(alloc), m_types(alloc), m_global_variables(alloc), m_funcs(alloc)
    {
    }


    SyntaxAnalyzer::~SyntaxAnalyzer()
    {
    }


    bool SyntaxAnalyzer::runPass(PreCompile* pc)
    {
        m_pc = pc;

        if(m_node->getType() != ASTNodeBase::ASTNodeTypes::AST_BLOCK) {
            pc->addError("invalid entry type", this);
            return false;
        }
        
        ASTBlockNode* rootBlock = (ASTBlockNode*)m_node;
        auto types = rootBlock->pick(ASTBlockNode::SortType::STRUCT_DECLEAR);
        //TODO
        
        auto funcs = rootBlock->pick(ASTBlockNode::SortType::FUNC_DECLEAR);
        
        bool cp = true;
        for(ASTNodeBase* fun : funcs) {
            ASTFuncDefNode* func = (ASTFuncDefNode*)fun;
            cp = func->registerTo(this);
        }
        if(!cp) return cp;


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
        LogDebug("[Analyzer] Global variables: ");
        for (auto it = m_global_variables.begin(); it != m_global_variables.end(); ++it) {
            LogDebug("\t VarName: ", it.key(), " VarType: ", it.value()->getRawTypeStr());
        }
        LogDebug("[Analyzer] Functions");
        for (auto func : m_funcs) {
            BeginAppender();
            for (auto* tp : func.m_types) {
                AppenderAppend(tp->getRawTypeStr());
                AppenderAppend(",");
            }
            std::string typesStr = EndAppender();

            LogDebug("\t Name: ", func.name, "(", typesStr.substr(0, typesStr.size() - 1), ") : ", func.m_ret_type->getRawTypeStr());
        }
    }

}