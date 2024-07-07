#include "PreCompile.hpp"
#include "base/Logger.hpp"

namespace cal {

    PreCompile::PreCompile(IAllocator& alloc)
        : m_errors(alloc), m_passes(alloc)
    {
    }


    void PreCompile::addPass(IPrecompilePass* pass)
    {
        // if (m_passes.indexOf(pass)) return;
        m_passes.push(pass);
    }


    void PreCompile::delPass(IPrecompilePass* pass)
    {
        m_passes.eraseItem(pass);
        m_passes.removeDuplicates();
    }


    IPrecompilePass* PreCompile::getPass(i32 idx) const
    {
        if (idx < 0 || idx >= m_passes.size()) return nullptr;
        return m_passes[idx];
    }


    bool PreCompile::run(ASTNodeBase* node)
    {
        // Logic
        if (m_passes.size() == 0) {
            LogWarn("Analyser found no pass in queue :(");
            return false;
        }

        for (auto* pass : m_passes) {
            pass->begin(node);
            auto var = pass->runPass(this);
            if(!var) {
                pass->end();
                LogError(pass->getName(), " run pass error");
                break;
            }
            pass->end();
        }

        return hasAnyError();
    }


    void PreCompile::printErrors()
    {
        if (m_errors.size() == 0) return;
        for (auto& err : m_errors) {
            if (err.level == PCError::ERROR) 
                LogError("[", err.pass->getName(), "] ", err.content);
        }
    }


    void PreCompile::printInfos()
    {
        if (m_errors.size() == 0) return;
        for (auto& err : m_errors) {
            if (err.level == PCError::INFO) 
                LogError("[", err.pass->getName(), "]", err.content);
        }
    }


    void PreCompile::printWarns()
    {
        if (m_errors.size() == 0) return;
        for (auto& err : m_errors) {
            if (err.level == PCError::WARN) 
                LogError("", err.pass->getName(), "]", err.content);
        }
    }


    void PreCompile::debugPrints()
    {
        //TODO
        for(auto* pass : m_passes) {
            pass->debugPrint();
        }
    }


    void PreCompile::addError(const std::string& str, IPrecompilePass* pass)
    {
        //ASSERT(pass);
        // if (!m_passes.indexOf(pass)) return;
        PCError err{
            .pass = pass,
            .content = str,
            .level = PCError::ERROR
        };
        m_errors.push(err);
    }


    void PreCompile::addWarn(const std::string& str, IPrecompilePass* pass)
    {
        ASSERT(pass);
        // if (!m_passes.indexOf(pass)) return;
        PCError err{
            .pass = pass,
            .content = str,
            .level = PCError::WARN
        };
        m_errors.push(err);
    }


    void PreCompile::addInfo(const std::string& str, IPrecompilePass* pass)
    {
        ASSERT(pass);
        // if (!m_passes.indexOf(pass)) return;
        PCError err{
            .pass = pass,
            .content = str,
            .level = PCError::INFO
        };
        m_errors.push(err);
    }


    bool PreCompile::hasAnyError() const
    {
        if (m_errors.size() == 0) return false;
        for (auto& err : m_errors) {
            if (err.level == PCError::ERROR)
                return true;
        }
        return false;
    }


}