#include "ASTStructNode.hpp"

#include "base/Logger.hpp"
#include "cal/ast/ASTNodeBase.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"
#include <stdexcept>
#include <json/json.h>

namespace cal {

    ASTStructNode::ASTStructNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_members(alloc)
    {
    }


    Json::Value ASTStructNode::buildOutput() const
    {
        Json::Value root(Json::ValueType::objectValue);
        Json::Value members(Json::ValueType::arrayValue);

        for (auto it = m_members.begin(); it != m_members.end(); ++it) 
        {
            if (!it.isValid()) continue;
            Json::Value argObj(Json::ValueType::objectValue);

            argObj["name"] = it.key();
            argObj["type"] = it.value().type->buildOutput();
            argObj["accessibility"] = it.value().accessibility;
            argObj["initialValue"] = it.value().defaultValue == nullptr ? "null" : it.value().defaultValue->buildOutput();

            members.append(argObj);
        }

        root["type"] = getNodeName();
        root["structName"] = m_name;
        root["structArgs"] = members;

        return root;
    }


    bool ASTStructNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        bool rs = true;
        for(auto& member : m_members) {
            rs |= member.type->checkSyntax(analyzer);
            if (member.defaultValue) {
                rs |= member.defaultValue->checkSyntax(analyzer);
            }
        }

        return rs;
    }


    std::string ASTStructNode::toString() const
    {
        StringBuilder members {};
        for (auto it = m_members.begin(); it != m_members.end(); ++it) 
        {
            if (!it.isValid()) continue;
            members.appendAll(
                " [Name:", it.key(), "]",
                " [Type:", it.value().type->toString(), "]", 
                " [AccessFlags:", it.value().accessibility, "]",
                " [InitialValue:", it.value().defaultValue == nullptr ? "null" : it.value().defaultValue->toString(), "] "
            );
        }

        return StringBuilder {
            ASTNodeBase::toString(),
            " [Name:", m_name, "]",
            " [Args(", m_members.size(), "):{", members, "}]"
        };
    }


    void ASTStructNode::set(const std::string& name)
    {
        if (!m_name.empty()) {
            LogWarn("Maybe duplicate!");
        }
        m_name = name;
    }


    void ASTStructNode::addMember(const std::string& var_name, ASTTypeNode* type, StructMemberAccessibility ass)
    {
        if(m_members.find(var_name).isValid()) {
            return;
        }
        if (type == nullptr) {
            throw std::runtime_error("struct member's type shouldn't be empty");
        }

        StructMember member {
            .accessibility = ass,
            .type = type,
            .defaultValue = nullptr
        };
        m_members.insert(var_name, member);

    }


    void ASTStructNode::removeMember(const std::string& var_name)
    {
        if (!m_members.find(var_name).isValid()) 
            return;
        m_members.erase(var_name);
    }


    void ASTStructNode::clearMember()
    {
        m_members.clear();
    }


    bool ASTStructNode::registerTo(SyntaxAnalyzer* analyzer)
    {
        if (analyzer->m_table->getStruct(m_name) != SymbolTable::INVALID_STRUCT) {
            analyzer->m_pc->addError("There are same name struct deacler before this", analyzer);
            return false;
        }

        SymbolTable::StructSymbol symbol {
            .content = this,
            .name = m_name,
            .selfType = ASTTypeNode::getTypeFromPool(m_name)
        };

        analyzer->m_table->addStruct(m_name, symbol);

        if (!analyzer->findTypeByTypeObj(symbol.selfType)) {
            analyzer->insertType(symbol.selfType);
        } else {
            analyzer->m_pc->addError("Some error lead that current struct's had been declear before struct declear", analyzer);
            return false;
        }

        return true;
    }


    ASTTypeNode* ASTStructNode::getMemberType(const std::string& name) const
    {
        if (!m_members.find(name).isValid()) 
            return nullptr;
        return m_members[name].type;
    }


    StructMemberAccessibility ASTStructNode::getMemberAccessibility(const std::string& name) const
    {
        if (!m_members.find(name).isValid()) 
            return DEFAULT;
        return m_members[name].accessibility;
    }


}