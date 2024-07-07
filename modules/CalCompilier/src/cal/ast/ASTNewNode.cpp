#include "ASTNewNode.hpp"

#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include "utils/StringBuilder.hpp"
#include <stdexcept>

namespace cal {

    ASTNewNode::ASTNewNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_initial_values(alloc)
    {
    }


    ASTNewNode::~ASTNewNode()
    {
        m_initial_values.clear();
    }


    Json::Value ASTNewNode::buildOutput() const
    {
        Json::Value root(Json::ValueType::objectValue);

        root["type"] = getNodeName();
        root["newType"] = m_name;
        root["realType"] = m_insType == nullptr ? "null" : m_insType->buildOutput();

        return root;
    }


    std::string ASTNewNode::toString() const
    {
        return StringBuilder {
            ASTNodeBase::toString(),
            " [NewType:", m_name, "]",
            " [RealType:", m_insType == nullptr ? "null" : m_insType->toString(), "]"
        };
    }


    ASTTypeNode* ASTNewNode::getReturnType() const
    {
        if (m_insType == nullptr) {
            throw std::runtime_error("before type analyze, you shouldn't call this");
        }
        return m_insType;
    }


    void ASTNewNode::set(const std::string& name) {
        m_name = name;
        m_insType = ASTTypeNode::getTypeFromPool(name);
    }


    void ASTNewNode::setInitialValue(const std::string& member_name, ASTNodeBase* value)
    {
        ASSERT(value);

        if (!m_initial_values.find(member_name).isValid()) {
            m_initial_values[member_name] = value;
            return;
        }

        m_initial_values.insert(member_name, value);
    }


    void ASTNewNode::removeInitialValueSet(const std::string& member_name)
    {
        if (!m_initial_values.find(member_name).isValid()) return;
        m_initial_values.erase(member_name);
    }


    void ASTNewNode::clearInitialValueSet()
    {
        m_initial_values.clear();
    }

}