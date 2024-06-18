#include "Utils.hpp"

#include <json/json.h>
#include <fstream>

namespace cal::utils {

    std::string buildOutputFromJson(const Json::Value& val, bool compact) {
        Json::StreamWriterBuilder writer;
        if (compact) {
            writer["commentStyle"] = "None";
            writer["indentation"] = "";
        }

        std::string jsonString = Json::writeString(writer, val);
        return jsonString;
    }


    bool writeOutputFromJson(const Json::Value& val, const std::string& fn, bool compact) {
        Json::StreamWriterBuilder writer;
        if (compact) {
            writer["commentStyle"] = "None";
            writer["indentation"] = "";
        }

        std::string jsonString = Json::writeString(writer, val);
        
        std::ofstream outFile(fn);
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
} // namespace cal::utils