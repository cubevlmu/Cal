#include "ASTTypeNode.hpp"

#include "base/Utils.hpp"
#include "base/allocator/Allocators.hpp"
#include "base/allocator/IAllocator.hpp"
#include "base/allocator/TagAllocator.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"
#include <cctype>
#include <json/json.h>

namespace cal {

    static const char* TypeStateStr[] = {
        "i32", "i64", "i16", "i8", "f32", "boolean",
        "u32", "u64", "u16", "u8", "f64", "null", "defined", "none", "void"
    };


    static int findTypeState(const std::string& type) {
        const char** begin = std::begin(TypeStateStr);
        const char** end = std::end(TypeStateStr);
        const char** it = std::find(begin, end, type);

        if (it != end) {
            return std::distance(begin, it);
        }
        else {
            return -1; // fallback index
        }
    }


    ASTTypeNode::ASTTypeNode(IAllocator& alloc)
        : ASTNodeBase(alloc), m_type_params(alloc)
    {
    }


    ASTTypeNode::ASTTypeNode(IAllocator& alloc, const std::string& typeStr)
        : ASTNodeBase(alloc), m_type_params(alloc)
    {
        setType(typeStr);
    }


    ASTTypeNode::ASTTypeNode(IAllocator& alloc, TypeState typeStr)
        : ASTNodeBase(alloc), m_type_params(alloc)
    {
        setType(typeStr);
    }


    ASTTypeNode::~ASTTypeNode() = default;


    Json::Value ASTTypeNode::buildOutput() const
    {
        Json::Value obj(Json::ValueType::objectValue);

        obj["type"] = ASTNodeBase::ASTNodeTypesString[(int)this->getType()];
        obj["rawTypeStr"] = m_raw_type;
        obj["isArray"] = m_is_array;
        obj["isVarArray"] = m_any_length_arr;
        if (m_is_array) {
            Json::Value params(Json::ValueType::arrayValue);

            for (auto& itm : m_type_params) {
                params.append(itm);
            }

            obj["arrayParams"] = params;
        }

        return obj;
    }


    std::string ASTTypeNode::toString() const
    {
        StringBuilder args {};
        for (auto& itm : m_type_params)
            args.appendAll(itm, " ");
        
        return StringBuilder {
            ASTNodeBase::toString(),
            " [State:", TypeStateStr[(i32)m_state], "]",
            " [IsArray:", m_is_array, "]",
            " [Params(", m_type_params.size(), "):{", args, "}]"
            " [IsAArray:", m_any_length_arr, "]"
        };
    }


    void ASTTypeNode::setType(const std::string& type_Str)
    {
        size_t pos = 0, left_ = 0;
        bool isArray = false;

        do {
            if (type_Str[pos] == '[') {
                isArray = true;
                pos++;
                size_t param_start = pos;

                do {
                    if (type_Str[pos] == ',') {
                        auto param_str = type_Str.substr(param_start, pos - param_start);
                        if (pos - param_start == 0) {
                            m_type_params.push(-1);
                            m_any_length_arr |= true;
                            pos++;
                        }
                        else {
                            int param = (std::stoi(param_str));
                            m_type_params.push(param);
                        }

                        pos++;
                        param_start = pos;
                    }
                    else if (type_Str[pos] == ']') {
                        auto param_str = type_Str.substr(param_start, pos - param_start);
                        if (pos - param_start == 0) {
                            m_type_params.push(-1);
                            m_any_length_arr |= true;
                            pos++;
                        }
                        else {
                            int param = (std::stoi(param_str));
                            m_type_params.push(param);
                        }
                        break;
                    }
                    else if (!std::isdigit(type_Str[pos])) {
                        pos = param_start;
                        m_type_params.push(-1);
                        m_any_length_arr |= true;
                        //throw std::runtime_error("expected number in array defination '[x]'");
                    }
                } while (pos++ < type_Str.size());
            }
        } while (pos++ < type_Str.size());

        m_raw_type = type_Str;
        m_raw_type = m_raw_type.erase(0, m_raw_type.find_first_not_of(' '));
        if (m_any_length_arr) { isArray = true; }
        if (isArray) {
            if (type_Str[type_Str.size() - 1] != ']')
                throw std::runtime_error("array type has no end tag : ']'");
            m_is_array = isArray;
            
            m_origin_type_str = m_raw_type;
            m_raw_type = m_raw_type.erase(m_raw_type.find("["), m_raw_type.find("]"));
        }


        std::string r_a_type = m_raw_type;
        std::transform(r_a_type.begin(), r_a_type.end(), r_a_type.begin(), [](unsigned char c) { return std::tolower(c); });

        int r_result = findTypeState(r_a_type);

        if (r_result == -1) {
            m_state = TypeState::defined;
        }
        else {
            m_state = (TypeState)r_result;
        }

        m_is_verified = true;
    }


    void ASTTypeNode::setType(TypeState state) {
        m_raw_type = TypeStateStr[(int)state];
        m_state = state;
        m_is_verified = true;
    }


    bool ASTTypeNode::isVerified() const
    {
        return m_is_verified;
    }


    void ASTTypeNode::clear()
    {
        m_is_verified = false;
        m_state = TypeState::none;
    }


    bool ASTTypeNode::isStanderType() const
    {
        return m_state != TypeState::null && m_state != TypeState::defined;
    }


    bool ASTTypeNode::compareType(ASTTypeNode* node) {
        // if (node->isStanderType() && isStanderType()) {
        //     return compareStanderType(node);
        // }
        return node->m_is_array == m_is_array &&
            node->m_is_verified == m_is_verified &&
            node->m_raw_type == m_raw_type &&
            node->m_type_params.size() == m_type_params.size() &&
            node->m_state == m_state;
    }


    bool ASTTypeNode::compareStanderType(ASTTypeNode* node)
    {
        // this is useless
        // i32 ms = (i32)node->m_state;
        // i32 ns = (i32)m_state;
        // if((ms >= 11 || ms == 5) && (ns >= 11 || ns == 5) ) {
        //     return ms == ns;
        // }

        return false;
    }


    bool ASTTypeNode::checkSyntax(SyntaxAnalyzer* analyzer) const
    {
        if (isAnyLengthArray() && m_allow_any_length_arr == false) {
            analyzer->m_pc->addError("variable should declear array length at here [x,x]", analyzer);
            return false;
        }
        if (isStanderType())
            return true;

        if (!analyzer->findTypeByRawType(m_raw_type)) {
            analyzer->m_pc->addError("variable type is not been declear", analyzer);
            return false;
        }

        return true;
    }

    class ASTTypePool : public ASTNodePool<ASTTypeNode>
    {
    public:
        ASTTypePool(IAllocator& alloc) : ASTNodePool(alloc, "TypeNodePool") {}
    };

    static ref<ASTTypePool> s_Pool;


    ASTTypeNode* ASTTypeNode::getTypeFromPool(const std::string& raw_type)
    {
        if (!s_Pool) {
            s_Pool.reset(CAL_NEW(getGlobalAllocator(), ASTTypePool)(getGlobalAllocator()));
        }

        ASTTypeNode* node = s_Pool->newToPool(raw_type);
        if (!node->isVerified())
            node->setType(raw_type);

        return node;
    }


    ASTTypeNode* ASTTypeNode::getTypeFromPool(TypeState state)
    {
        if (!s_Pool) {
            s_Pool.reset(CAL_NEW(getGlobalAllocator(), ASTTypePool)(getGlobalAllocator()));
        }
        
        ASTTypeNode* node = s_Pool->newToPool(TypeStateStr[(i32)state]);
        if (!node->isVerified())
            node->setType(state);

        return node;
    }
}