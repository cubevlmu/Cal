#include "ASTOPNode.hpp"

#include <json/json.h>
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

namespace cal {

    static const char* OperatorTypeStr[] = {
        "Add", "Subtract", "Multiply", "Divide", "Modulus",
        "And", "Or", "Not",
        "Equal", "NotEqual",
        "LessThan", "GreaterThan",
        "LessOrEqual", "GreaterOrEqual"
    };


    ASTOPNode::ASTOPNode(IAllocator& alloc)
        : ASTNodeBase(alloc)
    {
    }


    ASTOPNode::~ASTOPNode() = default;


    Json::Value ASTOPNode::buildOutput() const
    {
        Json::Value obj(Json::ValueType::objectValue);

        obj["type"] = ASTNodeBase::ASTNodeTypesString[(i32)getType()];
        obj["op"] = OperatorTypeStr[(i32)m_type];
        obj["left"] = m_left == nullptr ? "nil" : m_left->buildOutput();
        obj["right"] = m_right == nullptr ? "nil" : m_right->buildOutput();

        return obj;
    }


    std::string ASTOPNode::toString() const
    {
        return StringBuilder {
            ASTNodeBase::toString(),
            " [OP:", OperatorTypeStr[(i32)m_type], "]",
            " [Left:", m_left == nullptr ? "nil" : m_left->toString(), "] ",
            " [Right:", m_right == nullptr ? "nil" : m_right->toString(), "]"
        };
    }


    ASTTypeNode* ASTOPNode::getReturnType() const
    {
        ASSERT(m_left);
        ASSERT(m_right);

        // if (!m_left->getReturnType()->compareType(m_right->getReturnType()))
        //     return nullptr;
        return m_left->getReturnType();
    }


    void ASTOPNode::set(const OperatorType& type, ASTNodeBase* left, ASTNodeBase* right)
    {
        ASSERT(left);
        ASSERT(right);

        m_left = left;
        m_right = right;
        m_type = type;
    }


    bool ASTOPNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        bool pc = m_left->checkSyntax(analyzer);
        if (!pc) return pc;
        pc = m_right->checkSyntax(analyzer);
        if (!pc) return pc;

        ASTTypeNode* lt = m_left->getReturnType();
        ASTTypeNode* rt = m_right->getReturnType();
        if (lt == nullptr) {
            analyzer->m_pc->addError("left oprand's type is unknown", analyzer);
            return false;
        }
        if (rt == nullptr) {
            analyzer->m_pc->addError("right oprand's type is unknown", analyzer);
            return false;
        }

        if (!lt->compareType(rt)) {
            analyzer->m_pc->addError("two oprand's type is not equal", analyzer);
            return false;
        }

        return true;
    }

}