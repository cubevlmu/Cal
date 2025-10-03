// Created by cubevlmu on 2025/8/15.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>

#include <unordered_map>
#include <string>

namespace neo {

    /// Command line arguments parser
    /// --[FLAG][=VALUE] or -[FLAG][=VALUE] supporting boolean switch and string value
    class NCmdParser final
    {
        union CmdFlag {
            bool* bFlag;
            std::string* sFlag;
        };

    public:
		/// Construct CmdParser with size of arguments and argument array pointer
        NCmdParser(i32 size, char** argv);
        ~NCmdParser();

        /// Return false if any syntax result happend
        bool parse();

        /// Register boolean typed receiver
        void regBool(const char* flag, bool*);
        /// Register string typed receiver
        void regStr(const char* flag, std::string&);

    private:
        std::unordered_map<std::string_view, CmdFlag> m_flags;
        std::string m_cmdLine;
    };

}