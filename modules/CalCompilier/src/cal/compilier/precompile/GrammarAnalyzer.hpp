#pragma once

#include "PreCompile.hpp"
#include "base/allocator/IAllocator.hpp"

namespace cal {
    
    class GrammarAnalyzer : public IPrecompilePass
    {
    public:
        GrammarAnalyzer(IAllocator& alloc);
        ~GrammarAnalyzer() = default;

        virtual bool runPass(PreCompile* pc) override;
        virtual void begin(ASTNodeBase* ast) override;
        virtual void end() override;

        virtual std::string getName() const override { return "Grammar"; }

    private:
        IAllocator& m_alloc;
    };
}