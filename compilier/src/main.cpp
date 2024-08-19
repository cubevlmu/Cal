#include <iostream>
#include <fstream>

#include "analyzer/Lexer.hpp"
#include "analyzer/ast/NodeBase.hpp"
#include "analyzer/ast/expr/NumberNode.hpp"
#include "analyzer/ast/types/NodeType.hpp"
#include "base/allocator/Allocator.hpp"

#include "analyzer/ast/types/TypePool.hpp"

#include <globals.hpp>
#include <base/Logger.hpp>
#include <ostream>

namespace cal {

    i32 compilier_main(i32 argc, char** argv) {
        InitLogger();

        static Allocator global{};

        while (true) {
            LogInfo("> Input number to parse (exit for quit):");
            std::cin.clear();
            std::string val;
            std::getline(std::cin, val);

            if (val == "exit") {
                LogInfo("Exiting...");
                break;
            }

            auto* node = NumberPool::get().getNum(val);
            if (node == nullptr) continue;
            std::string output = ASTNodeBase::buildOutputJson(node);
            LogInfo(output, "\n");
        }

        return 0;

        const char* code = R"(
            import std.io;

            struct test {
                a : i32
            }

            fun main() {
                val tt = test();
                tt.a = 10;
                console.println(tt.a);
            }
        )";

        Lexer lex{ code, global };
        lex.analyze();
        lex.debugPrint();

        std::string output = lex.buildOutput();

        std::ofstream stream("lexer_output.txt");
        if (!stream.is_open()) {
            LogError("Failed to write lexer output");
            return -1;
        }
        stream << output;
        stream.flush();
        stream.close();

        return 0;
    }
}


int main(int argc, char** argv) {
    return cal::compilier_main(argc, argv);
}