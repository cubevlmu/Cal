#include "NumberNode.hpp"

#include "analyzer/ast/NodeBase.hpp"
#include "analyzer/ast/types/NodeType.hpp"
#include "analyzer/ast/types/TypePool.hpp"
#include "base/allocator/Allocators.hpp"
#include "globals.hpp"

#include <cctype>
#include <cmath>
#include <json/json.h>

namespace cal {

    ASTNumberNode::ASTNumberNode(IAllocator& alloc)
        : ASTExprNode(alloc)
    {
        m_currentPreferedType = number_type::unknown;
    }


    ASTNumberNode::ASTNumberNode(IAllocator& alloc, const number_val& val, number_type type)
        : ASTExprNode(alloc)
    {
        m_numberStorge = val;
        m_currentPreferedType = type;
    }


    bool ASTNumberNode::parse(const std::string& number_str)
    {
        if (number_str.empty())
            return false;

        bool isHex = false;
        bool isFloat = false;
        bool hasSuffix = false;
        char suffix = ' ';

        u32 idx = 0;
        u32 max = number_str.size();
        do {
            char current = number_str[idx];
            if (current == 'X' || current == 'x') {
                if (idx != 1) {
                    ASTError("invalid char found in number -> ", number_str, " at -> ", idx);
                    return false;
                }
                if (number_str[0] != '0') {
                    ASTError("hex number but not start with 0 -> ", number_str);
                    return false;
                }
                if (max < 3) {
                    ASTError("hex number but too short -> ", number_str);
                    return false;
                }
                isHex = true;
            }
            if (current == '.') {
                if (isFloat) {
                    ASTError("multiple dot in one number -> ", number_str);
                    return false;
                }
                if (isHex) {
                    ASTError("find . in hex number -> ", number_str);
                    return false;
                }
                isFloat = true;
                continue;
            }
            if (idx == max - 1) {
                if (!std::isdigit(current)) {
                    suffix = current;
                    hasSuffix = true;
                    continue;
                }
            }
            if (!std::isdigit(current)) {
                ASTError("invalid number -> ", number_str, " at -> ", idx);
                return false;
            }
        } while (idx++ < max - 1);

        try {
            if (isHex) {
                i64 val = std::stoll(number_str, nullptr, 16);
                if (val <= INT_MAX) {
                    m_numberStorge.i32 = static_cast<i32>(val);
                    m_currentPreferedType = number_type::I32;
                }
                else {
                    m_numberStorge.i64 = val;
                    m_currentPreferedType = number_type::I64;
                }

                m_isVerified = true;
                return true;
            }


            if (isFloat) {
                if (hasSuffix) {
                    if (suffix == 'f' || suffix == 'F') {
                        double d = std::stod(number_str);
                        float f32_val = static_cast<float>(d);

                        if (fabs(d - f32_val) < std::numeric_limits<float>::epsilon() && d <= kMaxF32) {
                            m_numberStorge.f32 = static_cast<float>(d);
                            m_currentPreferedType = number_type::F32;
                        }
                        else {
                            ASTError("trying to force use f32 to store -> ", number_str, " but it need f64 to store");
                            return false;
                        }

                        m_numberStorge.f32 = static_cast<float>(d);
                        m_currentPreferedType = number_type::F32;
                    }
                    else if (suffix == 'd' || suffix == 'D') {
                        double d = std::stod(number_str);
                        m_numberStorge.f64 = d;
                        m_currentPreferedType = number_type::F64;
                    }
                    else {
                        ASTError("unknown suffix behind float dot -> ", suffix, " at -> ", number_str);
                        return false;
                    }
                }
                else {
                    double val = std::stod(number_str);
                    float f32_val = static_cast<float>(val);

                    if (fabs(val - f32_val) < std::numeric_limits<float>::epsilon() && val <= kMaxF32) {
                        m_numberStorge.f32 = static_cast<float>(val);
                        m_currentPreferedType = number_type::F32;
                    }
                    else {
                        m_numberStorge.f64 = val;
                        m_currentPreferedType = number_type::F64;
                    }
                }

                m_isVerified = true;
                return true;
            }

            if (hasSuffix) {
                if (suffix == 'L' || suffix == 'l') {
                    long long val = std::stoll(number_str);
                    m_numberStorge.i64 = val;
                    m_currentPreferedType = number_type::I64;
                }
                else {
                    ASTError("unknown suffix after number -> ", suffix, " at -> ", number_str);
                    return false;
                }
            }
            else {
                long val = std::stol(number_str);
                if (val <= kMaxI32) {
                    m_numberStorge.i32 = static_cast<i32>(val);
                    m_currentPreferedType = number_type::I32;
                }
                else {
                    m_numberStorge.i64 = val;
                    m_currentPreferedType = number_type::I64;
                }
            }

            m_isVerified = true;
            return true;
        }
        catch (...) {
            ASTError("failed to parse number -> ", number_str);
            return false;
        }

        return false;
    }



    ASTNodeType* ASTNumberNode::returnType()
    {
        std::string str = ASTNodeType::typeToStr((ASTNodeType::Types)m_currentPreferedType);
        return TypePool::get().getType(str);
    }


    Json::Value ASTNumberNode::buildOutput()
    {
        Json::Value val = ASTNodeBase::buildOutput();
        val["NumberType"] = ASTNodeType::typeToStr((ASTNodeType::Types)m_currentPreferedType);

        switch (m_currentPreferedType) {
        case unknown:
            val["NumberValue"] = "unknown";
            break;
        case I8:
        case I16:
        case I32:
        case U8:
        case U16:
        case U32:
            val["NumberValue"] = m_numberStorge.i32;
            break;
        case U64:
        case I64:
            val["NumberValue"] = std::to_string(m_numberStorge.i64);
            break;
        case F32:
            val["NumberValue"] = m_numberStorge.f32;
            break;
        case F64:
            val["NumberValue"] = m_numberStorge.f64;
            break;
        }
        val["Type"] = m_numberType == nullptr ? "null" : m_numberType->buildOutput();

        return val;
    }

    //////////////////////////////////////////////
    // Number pool
    //////////////////////////////////////////////

    NumberPool::NumberPool()
        : m_pool(getGlobalAllocator()),
        m_alloc(getGlobalAllocator())
    {
    }


    NumberPool::~NumberPool()
    {
        for (auto item : m_pool) {
            CAL_DEL(m_alloc, item);
        }
        //  ASTDebug("number pool cleared : ", m_pool.size());
        m_pool.clear();
    }


    ASTNumberNode* NumberPool::getNum(const std::string& number_string)
    {
        if (number_string.empty())
            return nullptr;

        auto idx = m_pool.find(number_string);
        if (idx.isValid()) {
            return idx.value();
        }

        ASTNumberNode* node = CAL_NEW(m_alloc, ASTNumberNode)(m_alloc);
        if (!node->parse(number_string)) {
            ASTWarn("failed to parse number -> ", number_string);
            CAL_DEL(m_alloc, node);
            return nullptr;
        }

        m_pool.insert(number_string, node);
        // ASTDebug("number pool insert : ", number_string, " ptr : ", (long)node);
        return node;
    }


    void NumberPool::clear()
    {
        for (auto item : m_pool) {
            CAL_DEL(m_alloc, item);
        }
        //  ASTDebug("number pool cleared : ", m_pool.size());
        m_pool.clear();
    }
}