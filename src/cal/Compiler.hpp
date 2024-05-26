#pragma once

namespace cal {

    class Parser;

    class Compiler {
    public:
        Compiler(Parser& parser);

        void compile();
        void debugPrint();

    private:
        Parser& m_parser;
    };
}