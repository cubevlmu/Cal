#include <cstring>
#include <exception>
#include <iostream>

#include "base/Logger.hpp"

#include <base/allocator/Allocators.hpp>
#include "cal/compilier/Lexer.hpp"
#include "cal/compilier/Parser.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include "cal/compilier/precompile/PreCompile.hpp"
#include "cal/compilier/precompile/SyntaxAnalyzer.hpp"
#include "utils/StringBuilder.hpp"

#include "system/io/Stream.hpp"
#include "system/SysIO.hpp"

// #include "cal/Lexer.hpp"
// #include "cal/Parser.hpp"
// #include "cal/TypeChecker.hpp"
// #include "cal/CodeGenerator.hpp"
// #include "cal/bytecode/ByteCode.hpp"

int main(int argc, char** argv) {
    InitLogger();

    if (argc > 0) {
        // Run the script from args
        LogInfo("[Arguments] catched:");
        for (int i = 0; i < argc; i++) {
            LogInfo("\t[", i, "] ", argv[i]);
        }
        LogInfo("");
    }
    else {
        // Go to command line mode
    }

    static cal::Allocator alloc{};

    // cal::CalBytecode code {};
    // code.read("test.calcls");
    // code.debugPrint();

    // return 0;

    std::string test_source = R"(
        /* 
        import std.network.socket;

        import std.io.file;
        export fun test1() {
            test();
        }
        export 'c' var abc : i32;
        extern {
            fun nativePrint(a : char[]);
            fun nativeRead() : char[];
        }
       
        module main; 
        */

        struct test {
            a : i32,
            b : const i32,
            c : internal i32[1]
        }

        struct string {
            str : internal u16[8],
            len : internal u64
        }

        var dd : i32;
        
        fun main(args : string[]) {
            var c : i32;
            var a = 0;
            var b : i32 = 10;
            var e = new test;
            // e.a = 10;

            a = 10 + 20 * (11 + 12); 
            b = test();
            // print(a*b);
        }

        fun test() : i32 { 
            var a = 10;
            var b = 100;
            //return 10; 
        }
    )";

    cal::Lexer lex(test_source, alloc);
    lex.analyze();
    lex.debugPrint();

    cal::Parser parser(lex, alloc);
    parser.parse();
    parser.debugPrint();

    LogDebug("[Parser] AST Content:");
    LogDebug("\t", parser.getAST()->toString());


    cal::SyntaxAnalyzer* syntaxPass = CAL_NEW(alloc, cal::SyntaxAnalyzer)(alloc);

    cal::PreCompile pcmp(alloc);
    pcmp.addPass(syntaxPass);
    pcmp.run(parser.getAST());
    pcmp.printErrors();
    pcmp.debugPrints();

    // syntaxPass->m_table->dumpMap();

    // cal::CodeGenerator code_gen(parser);
    // code_gen.compile();
    // code_gen.debugPrint();

    // cal::CalBytecode codes = code_gen.getBytecodes();
    // codes.write("test.calcls");

    //cal::waitAnyKeyToExit();
    return 0;
}