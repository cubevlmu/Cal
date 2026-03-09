/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <nbase/common.hpp>
#include <nbase/memory/Memory.hpp>

namespace neo {

    /// Command line arguments parser
    /// --[FLAG][=VALUE] or -[FLAG][=VALUE] supporting boolean switch and string value
    class NCmdParser final
    {
        union CmdFlag {
            bool* bFlag;
            String* sFlag;
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
        void regStr(const char* flag, String&);

    private:
        HashMap<StringView, CmdFlag> m_flags;
	    String m_cmdLine;
    };

}