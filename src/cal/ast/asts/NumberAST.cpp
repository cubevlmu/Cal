#include "cal/ast/ASTNodes.hpp"

#include "base/Logger.hpp"
#include <json/json.h>
#include "globals.hpp"

namespace cal {


    NumberNode::NumberNode(const std::string& numberStr) {
        auto tp = checkType(numberStr);
        switch (tp)
        {
        case Float:
            this->f = std::stof(numberStr);
            break;
        case Double:
            this->d = std::stod(numberStr);
            break;
        case I32:
            this->i = std::stoi(numberStr);
            break;
        case I64:
            this->l = std::stoll(numberStr);
            break;
        case NotANumber:
        default:
            LogError("[Parser] Not a number -> ", numberStr);
            ASSERT(false);
        }
        this->number_type = tp;
    }


    Json::Value NumberNode::buildOutput() const {
        Json::Value value(Json::ValueType::objectValue);

        value["type"] = "Number";
        value["valueType"] = NumTypeChar[number_type];

        switch (number_type)
        {
        case Float:
            value["value"] = f;
        case Double:
            value["value"] = d;
        case I32:
            value["value"] = i;
        case I64:
            value["value"] = std::to_string(l);
        default: break;
        }
        
        return value;
    }


    NumberNode::NumType NumberNode::checkType(const std::string& str) {
        // Check if the string is empty or only contains whitespace
        if (str.empty() || std::all_of(str.begin(), str.end(), ::isspace)) {
            return NotANumber;
        }

        // Check for int
        try {
            size_t pos;
            long long intValue = std::stoll(str, &pos);
            if (pos == str.size()) {
                if (intValue <= std::numeric_limits<int>::max() && intValue >= std::numeric_limits<int>::min()) {
                    return I32;
                }
                else {
                    return I64;
                }
            }
        }
        catch (const std::exception&) {
            // Not an int
        }

        // Check for double
        try {
            size_t pos;
            double doubleValue = std::stod(str, &pos);
            if (pos == str.size()) {
                return Double;
            }
        }
        catch (const std::exception&) {
            // Not a double
        }

        return NotANumber;
    }
}