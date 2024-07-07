#pragma once

#include "base/allocator/IAllocator.hpp"
#include "base/types/Array.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "utils/StringBuilder.hpp"
namespace cal {

    class PreCompile;
    
    class IPrecompilePass 
    {
        friend class PreCompile;
    public:
        virtual ~IPrecompilePass() = default;

        virtual bool runPass(PreCompile* pc) = 0;
        virtual void begin(ASTNodeBase* ast) = 0;
        virtual void end() = 0;
        virtual void debugPrint(){};

        virtual std::string getName() const = 0;
    };


    class PreCompile
    {
        struct PCError
        {
            const IPrecompilePass* pass;
            const std::string content;
            const enum {
                WARN, ERROR, INFO
            } level;
        };
    public:
        PreCompile(IAllocator& alloc);

        void addPass(IPrecompilePass* pass);
        void delPass(IPrecompilePass* pass);

        IPrecompilePass* getPass(i32 idx) const;
        bool run(ASTNodeBase* node);

        void printErrors();
        void printInfos();
        void printWarns();
        void debugPrints();

        void addError(const std::string& str, IPrecompilePass* pass);
        void addWarn(const std::string& str, IPrecompilePass* pass);
        void addInfo(const std::string& str, IPrecompilePass* pass);

        bool hasAnyError() const;
        
    private:
        Array<IPrecompilePass*> m_passes;
        Array<PCError> m_errors;
    };
}