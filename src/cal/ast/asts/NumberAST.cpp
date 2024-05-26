#include "cal/ast/ASTNodes.hpp"

#include "base/Logger.hpp"
#include "base/Utils.hpp"
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
        case Int:
            this->i = std::stoi(numberStr);
            break;
        case Long:
            this->l = std::stoll(numberStr);
            break;
        case NotANumber:
        default:
            LogError("[Parser] Not a number -> ", numberStr);
            ASSERT(false);
            break;
        }
        this->number_type = tp;
    }


    std::string NumberNode::toString() const {
        BeginAppender();
        AppenderAppend("[NumberNode] \n\tType : ");
        AppenderAppend(NumTypeChar[number_type]);
        AppenderAppend(" \n\tValue : ");
        switch (number_type)
        {
        case Float:
            AppenderAppend(f);
            break;
        case Double:
            AppenderAppend(d);
            break;
        case Int:
            AppenderAppend(i);
            break;
        case Long:
            AppenderAppend(l);
            break;
        default: break;
        }
        return EndAppender();
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
                    return Int;
                }
                else {
                    return Long;
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