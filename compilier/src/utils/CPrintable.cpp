#include "CPrintable.hpp"

#include <json/json.h>
#include <string>

namespace cal {

    void CPrintable::debugPrint() const {}


    std::string CPrintable::buildOutput() const {
        return "";
    }


    Json::Value CPrintable::buildJson() const {
        return Json::Value();
    }
    
} // namespace cal