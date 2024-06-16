#include "ASTFuncDefNode.hpp"

#include "cal/ast/ASTBlockNode.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include <json/json.h>

namespace cal {

    ASTFuncArgNode::ASTFuncArgNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {

    }


    ASTFuncArgNode::~ASTFuncArgNode() = default;


    void ASTFuncArgNode::set(const std::string& name, ASTTypeNode* type) {
        m_name = name;
        m_type = type;

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



    ASTFuncDefNode::ASTFuncDefNode(IAllocator &alloc) 
        : ASTNodeBase(alloc), m_args(alloc)
    {

    }


    ASTFuncDefNode::~ASTFuncDefNode() = default;


    Json::Value ASTFuncDefNode::buildOutput() const {
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["funcName"] = m_name;
        root["funcArgs"];
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
        for(auto& argItem : m_args) {
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
        if(type == nullptr) 
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

} // namespace cal