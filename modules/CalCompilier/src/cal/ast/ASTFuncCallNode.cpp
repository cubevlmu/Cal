#include "ASTFuncCallNode.hpp"

#include <json/json.h>

namespace cal {

    ASTFuncCallNode::ASTFuncCallNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_args(alloc)
    {
    }


    ASTFuncCallNode::~ASTFuncCallNode() = default;


    Json::Value ASTFuncCallNode::buildOutput() const {
        Json::Value root(Json::ValueType::objectValue);
        Json::Value args(Json::ValueType::arrayValue);
        for (auto* arg : m_args) {
            args.append(arg->buildOutput());
        }

        root["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        root["funcName"] = m_name;
        root["funcArgs"] = args;

        return root;
    }


    void ASTFuncCallNode::setName(const std::string& name)
    {
        m_name = name;
    }


    void ASTFuncCallNode::addParam(ASTNodeBase* node) {
        if (node == nullptr)
            throw std::runtime_error("arg node can't be null");
        m_args.push(node);
    }


    void ASTFuncCallNode::delParam(ASTNodeBase* node) {
        if (node == nullptr)
            return;
        // if (m_args.find(node) == -1)
        //     return;
        m_args.eraseItem(node);
    }


    void ASTFuncCallNode::clearParams() {
        m_args.clear();
    }

} // namespace cal