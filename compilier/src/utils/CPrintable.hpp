#pragma once

#include <string>

namespace Json {
    class Value;
}

namespace cal {

    class CPrintable {
    public:
        virtual void debugPrint() const;
        virtual std::string buildOutput() const;
        virtual Json::Value buildJson() const;
    };
    
}