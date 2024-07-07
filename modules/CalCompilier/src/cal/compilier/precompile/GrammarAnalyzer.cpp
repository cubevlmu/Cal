#include "GrammarAnalyzer.hpp"

namespace cal {


    GrammarAnalyzer::GrammarAnalyzer(IAllocator& alloc)
        : m_alloc(alloc)
    {
    }


    bool GrammarAnalyzer::runPass(PreCompile* pc)
    {
        return false;
    }


    void GrammarAnalyzer::begin(ASTNodeBase* ast)
    {
    }


    void GrammarAnalyzer::end()
    {
    }


}