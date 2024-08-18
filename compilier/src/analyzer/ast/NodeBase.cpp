#include "NodeBase.hpp"

#include "analyzer/ast/NodeType.hpp"

#include <json/json.h>

namespace cal {

    std::string ASTNodeBase::buildOutputJson(ASTNodeBase* root) {
        auto r = root->buildOutput();
        std::string rr = r.toStyledString();
        return rr;
    }


    ASTNodeBase::ASTNodeBase(IAllocator& allocator)
        : m_alloc(allocator)
    {

    }

    Json::Value ASTNodeBase::buildOutput() {
        Json::Value value{ Json::ValueType::objectValue };
        value["type"] = getString(nodeType());

        return value;
    }

    std::string ASTNodeBase::toString() { return std::string(); }
}