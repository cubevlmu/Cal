/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "CmdParser.hpp"

#include "nbase/utils/StringUtils.hpp"

namespace neo {

    NCmdParser::NCmdParser(i32 argc, char** argv) {
        m_cmdLine.clear();
        for (auto idx = 1; idx < argc; idx++) {
            m_cmdLine.append(argv[idx]);
            m_cmdLine.append(" ");
        }
    }

    void NCmdParser::regBool(const char *flag, bool* bPtr) {
        if (m_flags.contains(flag)) {
            return;
        }
        m_flags.insert({flag, CmdFlag { .bFlag = bPtr }});
    }

    void NCmdParser::regStr(const char *flag, String& sPtr) {
        if (m_flags.contains(flag)) {
            return;
        }
        m_flags.insert({flag, CmdFlag {.sFlag = &sPtr}});
    }

    bool NCmdParser::parse() {
        psize idx = 0;
        const psize max = m_cmdLine.length();
        const StringView cmdLineView = m_cmdLine.c_str();

        while (idx < max) {
            char c = m_cmdLine[idx];

            // skip whitespace
            if (std::isspace(static_cast<unsigned char>(c))) {
                ++idx;
                continue;
            }

            if (c != '-') {
                return false; // invalid token start
            }

            ++idx; // skip '-'
            if (idx < max && m_cmdLine[idx] == '-') {
                // long option, skip second dash
                ++idx;
            }

            // parse flag name
            const psize start = idx;
            bool isQuote = false;
            bool checkFlag = false;
            StringView flag;
            String value;

            while (idx < max && (!std::isspace(m_cmdLine[idx]) || isQuote)) {
                if (m_cmdLine[idx] == '=') {
                    flag = cmdLineView.substr(start, idx - start);
                    checkFlag = true;
                    ++idx; // skip '='
                    break;
                }

                if (m_cmdLine[idx] == '\\' && idx + 1 < max && m_cmdLine[idx+1] == '\"') {
                    idx += 2;
                    continue;
                }

                if (m_cmdLine[idx] == '\"') {
                    isQuote = !isQuote;
                }

                ++idx;
            }

            if (isQuote) return false; // unclosed quote

            if (!checkFlag) {
                flag = cmdLineView.substr(start, idx - start);
            }

            // parse value if '=' was found
            if (checkFlag) {
                const psize valueStart = idx;
                while (idx < max && (!std::isspace(m_cmdLine[idx]) || isQuote)) {
                    if (m_cmdLine[idx] == '\\' && idx + 1 < max && m_cmdLine[idx+1] == '\"') {
                        idx += 2;
                        continue;
                    }
                    if (m_cmdLine[idx] == '\"') {
                        isQuote = !isQuote;
                    }
                    ++idx;
                }
                if (isQuote) return false;
                value = cmdLineView.substr(valueStart, idx - valueStart);
                replaceAll(value, "\"", "");
            }

            // assign to map
            auto it = m_flags.find(flag);
            if (it != m_flags.end()) {
                auto& v = it->second;
                if (checkFlag) {
                    *v.sFlag = value;
                } else {
                    *v.bFlag = true;
                }
            }
        }

        return true;
    }

    NCmdParser::~NCmdParser() {
        m_flags.clear();
        m_cmdLine.clear();
    }
}