/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "neo/Compiler.hpp"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#include <stdlib.h>

static void configureDebugCrt()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
}
#endif

/// Compiler main entrance
int main(int argc, char** argv) {
#if defined(_MSC_VER) && defined(_DEBUG)
    configureDebugCrt();
#endif
    neo::NCompiler cmp {argc, argv};
    return cmp.runCompiler();
}
