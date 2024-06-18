#include "ASTTypeNode.hpp"

#include "base/Utils.hpp"
#include <cctype>
#include <json/json.h>

namespace cal {

    static const char* TypeStateStr[] = {
        "i32", "i64", "i16", "i8", "f32", "boolean",
        "u32", "u64", "u16", "u8", "f64", "null", "defined", "none"
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


    ASTTypeNode::~ASTTypeNode() = default;


    Json::Value ASTTypeNode::buildOutput() const
    {
        Json::Value obj(Json::ValueType::objectValue);

        obj["type"] = ASTNodeBase::ASTNodeTypesString[(int)this->getType()];
        obj["rawTypeStr"] = m_raw_type;
        obj["isArray"] = m_is_array;
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
        std::string parent = ASTNodeBase::toString();
        BeginAppender();
        AppenderAppend(parent);
        AppenderAppend("\n\tTypeState: "); AppenderAppend(TypeStateStr[(int)m_state]);
        AppenderAppend("\n\tIsArray: "); AppenderAppend(m_is_array ? "true" : "false");
        AppenderAppend("\n\tArrayParams: ");
        for (auto& itm : m_type_params) {
            AppenderAppend(itm); AppenderAppend(" ");
        }
        return EndAppender();
    }


    void ASTTypeNode::setType(const std::string& type_Str)
    {
        size_t pos = 0, left_ = 0;
        bool isArray;

        do {
            if (type_Str[pos] == '[') {
                isArray = true;
                pos++;
                size_t param_start = pos;

                do {
                    if (type_Str[pos] == ',') {
                        auto param_str = type_Str.substr(param_start, pos - param_start);
                        int param = (std::stoi(param_str));
                        m_type_params.push(param);

                        pos++;
                        param_start = pos;
                    }
                    else if (type_Str[pos] == ']') {
                        auto param_str = type_Str.substr(param_start, pos - param_start);
                        int param = (std::stoi(param_str));
                        m_type_params.push(param);
                        break;
                    }
                    else if (!std::isdigit(type_Str[pos])) {
                        pos = param_start;
                        throw std::runtime_error("expected number in array defination '[x]'");
                    }
                } while (pos++ < type_Str.size());
            }
        } while (pos++ < type_Str.size());

        m_raw_type = type_Str;
        if (isArray) {
            if (type_Str[type_Str.size() - 1] != ']')
                throw std::runtime_error("array type has no end tag : ']'");
            m_is_array = isArray;
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


    bool ASTTypeNode::compareType(ASTTypeNode* node) {
        return node->m_is_array == m_is_array &&
            node->m_is_verified == m_is_verified &&
            node->m_raw_type == m_raw_type &&
            //todo node->m_type_params == m_type_params &&
            node->m_state == m_state;
    }
}