#include "ASTFuncDefNode.hpp"

#include "cal/ast/ASTBlockNode.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include <json/json.h>
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"

namespace cal {

    ////////////////////////////////////////////////////////////////////////
    /// AST Function Argument Node
    ////////////////////////////////////////////////////////////////////////

    ASTFuncArgNode::ASTFuncArgNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {

    }


    ASTFuncArgNode::~ASTFuncArgNode() = default;


    void ASTFuncArgNode::set(const std::string& name, ASTTypeNode* type) {
        m_name = name;

        // duplicate origin type for func args
        m_type = CAL_NEW(m_allocator, ASTTypeNode)(m_allocator);
        m_type->setType(type->getRawTypeStr());

        if (!type->isVerified())
            throw std::runtime_error("type not verified or not setup");
    }


    Json::Value ASTFuncArgNode::buildOutput() const {
        Json::Value obj(Json::ValueType::objectValue);

        obj["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        obj["argName"] = m_name;
        obj["argType"] = m_type->buildOutput();

        return obj;
    }


    void ASTFuncArgNode::registerTo(SyntaxAnalyzer* analyzer)
    {
    }


    bool ASTFuncArgNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        // such a fun design in ast :)
        m_type->setAllowAnyLengthArray(true);
        return m_type->checkSyntax(analyzer);
    }

    ////////////////////////////////////////////////////////////////////////
    /// AST Function Node
    ////////////////////////////////////////////////////////////////////////

    ASTFuncDefNode::ASTFuncDefNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_args(alloc)
    {

    }


    ASTFuncDefNode::~ASTFuncDefNode() = default;


    Json::Value ASTFuncDefNode::buildOutput() const {
        Json::Value root(Json::ValueType::objectValue);
        Json::Value args(Json::ValueType::arrayValue);

        for(auto* arg : m_args) {
            args.append(arg->buildOutput());
        }

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["funcName"] = m_name;
        root["funcArgs"] = args;
        root["funcRetType"] = m_ret_type->buildOutput();
        root["funcLinking"] = m_link_state;
        root["funcBody"] = m_func_body == nullptr ? "[null]" : m_func_body->buildOutput();

        return root;
    }


    void ASTFuncDefNode::setName(const std::string& name)
    {
        m_name = name;
    }


    void ASTFuncDefNode::addParams(ASTFuncArgNode* arg)
    {
        std::string name = arg->getArgName();
        for (auto& argItem : m_args) {
            if (argItem->getArgName() == name)
                throw std::runtime_error("argument has already add to this function");
        }

        m_args.push(arg);
    }


    void ASTFuncDefNode::delParams(ASTFuncArgNode* arg)
    {
        m_args.removeDuplicates();
        m_args.eraseItem(arg);
    }


    void ASTFuncDefNode::setRetType(ASTTypeNode* type)
    {
        if (type == nullptr)
            throw std::runtime_error("return type can't be null");
        if (!type->isVerified())
            throw std::runtime_error("return type not defined");

        m_ret_type = type;
    }


    void ASTFuncDefNode::setLinkType(FuncLinkType type)
    {
        if (type == FuncLinkType::DYNAMIC_BODY && m_func_body != nullptr)
            throw std::runtime_error("a dynamic linking function but using static declear body :(");
        m_link_state = type;
    }


    void ASTFuncDefNode::setBody(ASTBlockNode* body)
    {
        if (body == nullptr) {
            if (m_link_state == FuncLinkType::STATIC_BODY)
                throw std::runtime_error("function body shouldn't be null when linking mode is static");
        }

        m_func_body = body;
    }


    bool ASTFuncDefNode::registerTo(SyntaxAnalyzer* analyzer)
    {
        if (analyzer->m_now_func_state != nullptr) {
            analyzer->m_pc->addError("function mustn't declear in here", analyzer);
            return false;
        }

        auto fidx = analyzer->m_funcs.find(m_name);
        if (fidx.isValid()) {
            analyzer->m_pc->addError("function has been decleared before this", analyzer);
            return false;
        }

        bool cr = true;
        cr = m_ret_type->checkSyntax(analyzer);
        if (!cr) return cr;

        Array<std::string> arg_names(m_allocator);
        SyntaxAnalyzer::FuncDetail func{
            .m_ret_type = m_ret_type,
            .name = m_name
        };
        m_dtl = func;

        for (ASTFuncArgNode* func_arg : m_args) {
            i32 idx = arg_names.indexOf(func_arg->m_name);
            if (idx >= 0) {
                analyzer->m_pc->addError("current function's argument name repeated", analyzer);
                cr = false;
                break;
            }
            arg_names.push(func_arg->m_name);
            func.m_types.push_back(func_arg->m_type);
            cr |= func_arg->checkSyntax(analyzer);
        }
        if (!cr) return cr;
        analyzer->m_funcs.insert(m_name, func);

        return true;
    }


    bool ASTFuncDefNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        // body analyzing
        if (analyzer->m_now_func_state) {
            analyzer->m_pc->addError("seemed current function is in a function, we don't suppot closure", analyzer);
            return false;
            // CAL_DEL(analyzer->m_alloc, analyzer->m_now_func_state);
        }
        SyntaxAnalyzer::FuncDetail func {m_dtl};
        analyzer->m_now_func_state = CAL_NEW(analyzer->m_alloc, SyntaxAnalyzer::FuncState)(analyzer->m_alloc, func);

        bool cr = true;

        if (m_func_body == nullptr) {
            //TODO
        }
        else {
            cr = m_func_body->checkSyntax(analyzer);
        }

        CAL_DEL(analyzer->m_alloc, analyzer->m_now_func_state);
        analyzer->m_now_func_state = nullptr;
        if (!cr) return cr;
        // clean up & post declear

        return true;
    }

} // namespace cal