#include "ASTNodes.hpp"

#include <json/json.h>
#include <fstream>

namespace cal {

    //TODO
#define NumTypeCharSize 9

    const char* ASTBase::NumTypeChar[] = {
        "float", "double",
        "i32", "u32", "i64", "u64", "u8", "i8", "u16", "i16", "bool",
        "USER_DEFINED",
        "NotANumber", "ERR_TP",
        "FunctionReturn", "VariableReturn",
        "void"
    };


    ASTBase::NumType ASTBase::parseNumTypeText(const std::string& type) {
        const char* c_type = type.c_str();
        for (size_t i = 0; i <= NumTypeCharSize; i++) {
            if (strcmp(NumTypeChar[i], c_type) == 0)
                return NumType(i);
        }

        return NumType::ERR_TP;
    }


    std::string ASTBase::buildOutputFromJson(const Json::Value& val) {
        Json::StreamWriterBuilder writer;
        writer["commentStyle"] = "None";
        writer["indentation"] = "";

        std::string jsonString = Json::writeString(writer, val);
        return jsonString;
    }


    bool ASTBase::writeOutputToFile(
        const Json::Value& val,
        const std::string& file
    ) {
        Json::StreamWriterBuilder writer;
        std::string jsonString = Json::writeString(writer, val);

        std::ofstream outFile("ParseResult.json");
        if (outFile.is_open()) {
            outFile << jsonString;
            outFile.close();
            return true;
        }
        else {
            return false;
        }

        return true;
    }
} // namespace cal