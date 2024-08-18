#pragma once

#include "base/allocator/IAllocator.hpp"
#include "NodeType.hpp"
#include <base/Logger.hpp>

namespace Json {
    class Value;
}

#define ASTError(...) LogError("[AST] ", __VA_ARGS__)
#define ASTDebug(...) LogDebug("[AST] ", __VA_ARGS__)
#define ASTWarn(...) LogWarn("[AST] ", __VA_ARGS__)
#define ASTInfo(...) LogInfo("[AST] ", __VA_ARGS__)

namespace cal {

    class SymbolManager;
    class Analyzer;
    class CodeGenerator;

    class ASTNodeBase
    {
    public:
        static std::string buildOutputJson(ASTNodeBase*);

    public:
        ASTNodeBase(IAllocator& allocator);

        virtual ASTTypes nodeType() = 0;

        virtual Json::Value buildOutput();
        virtual std::string toString();

        virtual bool buildSymbol(SymbolManager& symbol) { return false; };
        virtual bool analyzeNode(Analyzer& analyzer) { return false; };
        virtual bool codeGen(CodeGenerator& gen) { return false; };

    protected:
        IAllocator& m_alloc;
    };

}