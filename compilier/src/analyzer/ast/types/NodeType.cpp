#include "NodeType.hpp"

#include <cctype>
#include <string>
#include "TypePool.hpp"
#include "utils/StringBuilder.hpp"

#include <json/json.h>

namespace cal {


    const char* ASTNodeType::TYPE_STRS[] = {
        "unknown",
        "i8", "i16", "i32", "i64",
        "u8", "u16", "u32", "u64",
        "f32", "f64",
        "bool",
        "custom"
    };


    int ASTNodeType::checkIdx(const std::string& type) {
        const char** begin = std::begin(ASTNodeType::TYPE_STRS);
        const char** end = std::end(ASTNodeType::TYPE_STRS);
        const char** it = std::find(begin, end, type);

        if (it != end) {
            return std::distance(begin, it);
        }
        else {
            return -1; // fallback index
        }
    }



    bool ASTNodeType::parse(const std::string& raw) {
        int idx = 0;
        int max = raw.size();

        int array_begin = -1;
        int array_end = -1;

        int template_begin = -1;
        int template_end = -1;

        do {
            char now = raw[idx];
            if (now == '[') {
                if (array_begin != -1) {
                    ASTError("found '[' duplicated! -> ", raw);
                    return false;
                }
                if ((template_begin != -1 && template_end == -1) || (template_begin != -1 && template_end != -1 && template_end >= idx)) {
                    continue;
                }
                array_begin = idx;
            }
            if (now == ']') {
                if ((template_begin != -1 && template_end == -1) || (template_begin != -1 && template_end != -1 && template_end >= idx)) {
                    continue;
                }
                if (array_begin == -1) {
                    ASTError("found ']' but not found '[' in a type define: ", raw);
                    return false;
                }
                array_end = idx;
            }
            if (now == '<') {
                template_begin = idx;
            }
            if (now == '>') {
                if (template_begin == -1) {
                    ASTError("found '<' but not found '>' in a type define: ", raw);
                    return false;
                }
                template_end = idx;
            }
        } while (idx++ < max);

        if ((array_begin != -1 && array_end == -1) || (array_begin != -1 && array_end >= max)) {
            ASTError("found '[' but not found ']' in current type define: ", raw);
            return false;
        }
        if (array_begin == -1 && array_end != -1) {
            ASTError("found ']' but not found '[' in current type define: ", raw);
            return false;
        }

        if (array_begin != -1 && array_end != -1 && array_end < max) {
            idx = array_begin + 1;
            int last_char = idx;
            int dimension_count = 0;

            do {
                char now = raw[idx];

                if (now == '[' || now == ']') {
                    ++idx;
                    continue;
                }

                if (now == ',' || idx == array_end - 1) {
                    if (idx == array_end - 1 && now != ',') {
                        ++idx;
                    }

                    int old = last_char;
                    last_char = idx;

                    std::string parm = raw.substr(old, last_char - old);
                    // parm.erase(parm.find_last_not_of(" \n\r\t") + 1);
                    // parm.erase(0, parm.find_first_not_of(" \n\r\t"));


                    if (parm.size() > 2 && (parm[0] == '0' && (parm[1] == 'x' || parm[1] == 'X'))) {
                        long value = std::stol(parm, nullptr, 16);
                        if (value < 0) {
                            ASTError("array lenght mustn't be smaller than 0 -> ", raw);
                            return false;
                        }
                        if (value <= INT_MAX) {
                            m_array_length_parms.push(array_length_parm{
                                .i = static_cast<uint32_t>(value),
                                .type = array_length_parm::I32
                                });
                        }
                        else {
                            m_array_length_parms.push(array_length_parm{
                                .l = static_cast<uint64_t>(value),
                                .type = array_length_parm::I64
                                });
                        }
                    }
                    else if (std::isdigit(parm[0])) {
                        long value = std::stol(parm);
                        if (value < 0) {
                            ASTError("array lenght mustn't be smaller than 0 -> ", raw);
                            return false;
                        }
                        if (value <= INT_MAX) {
                            m_array_length_parms.push(array_length_parm{
                                .i = static_cast<uint32_t>(value),
                                .type = array_length_parm::I32
                                });
                        }
                        else {
                            m_array_length_parms.push(array_length_parm{
                                .l = static_cast<uint64_t>(value),
                                .type = array_length_parm::I64
                                });
                        }
                    }
                    else {
                        m_array_length_parms.push(array_length_parm{
                            .c = parm.c_str(),
                            .type = array_length_parm::RefName
                            });
                    }

                    ++dimension_count;
                    last_char = idx + 1;
                }

                ++idx;

            } while (idx < array_end);

            m_array_dimension = dimension_count;
            m_isArray = true;
            if (dimension_count == 0) {
                m_array_ref = true;
            }
        }

        if (template_begin != -1 && template_end != -1 && template_end < max) {
            std::string parm = raw.substr(template_begin + 1, template_end - template_begin - 1);
            // parm.erase(parm.find_last_not_of(" \n\r\t") + 1);
            // parm.erase(0, parm.find_first_not_of(" \n\r\t"));

            ASTNodeType* type = TypePool::get().getType(parm);
            if (!type) {
                ASTError("Template can't be parsed -> ", parm, " from -> ", raw);
                return false;
            }

            m_template_type = type;
            m_isTemplate = true;
        }

        int type_begin = std::max(array_begin, template_begin);
        if (type_begin == -1) {
            m_type_str = m_raw_type_str;
        }
        else {
            m_type_str = m_raw_type_str.substr(0, type_begin);
        }

        int type_idx = checkIdx(m_type_str);
        if (type_idx <= 0 || type_idx == 12) {
            m_type = Types::custom;
        }
        else {
            m_type = (Types)type_idx;
        }

        m_isVerified = true;
        return true;
    }


    Json::Value ASTNodeType::buildOutput() {
        Json::Value root = ASTNodeBase::buildOutput();
        root["IsVerified"] = m_isVerified;
        root["ParsedType"] = typeToStr(m_type);
        root["IsArray"] = m_isArray;
        root["IsArrayAreReference"] = m_array_ref;
        root["ArrayDimension"] = m_array_dimension;

        Json::Value arrayLen{ Json::ValueType::arrayValue };
        for (auto item : m_array_length_parms) {
            switch (item.type)
            {
            case array_length_parm::RefName:
                arrayLen.append(item.RefName);
                break;
            case array_length_parm::I32:
                arrayLen.append(std::to_string(item.i));
                break;
            case array_length_parm::I64:
                arrayLen.append(std::to_string(item.l));
                break;
            }
        }
        root["ArrayLengthParms"] = arrayLen;

        root["IsTemplate"] = m_isTemplate;
        root["TemplateType"] = m_template_type == nullptr ? "nullptr" : m_template_type->buildOutput();

        root["TypeString"] = m_type_str;
        root["RawTypeString"] = m_raw_type_str;

        return root;
    }


    std::string ASTNodeType::toString() {
        return StringBuilder {

        };
    }


    bool ASTNodeType::isStanderType() {
        if (!m_isVerified)
            return false;
        int iType = (int)m_type;
        return iType > 0 && iType < 12;
    }

    bool ASTNodeType::isCustomType() {
        if (!m_isVerified)
            return false;
        return m_type == Types::custom;
    }


    bool ASTNodeType::compareType(ASTNodeType* type) {
        //TODO
        return false;
    }


    ASTNodeType::ASTNodeType(IAllocator& alloc, const std::string& type)
        : ASTNodeBase(alloc),
        m_array_length_parms(alloc)
    {
        m_raw_type_str = type;
    }


    ASTNodeType::ASTNodeType(IAllocator& alloc, Types type)
        : ASTNodeBase(alloc),
        m_array_length_parms(alloc)
    {
        //todo if(type != Types::custom || type != Types::custom) {

        // }
    }

} // namespace cal