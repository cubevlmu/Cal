#include <iostream>

#include "base/Logger.hpp"
#include "cal/Lexer.hpp"
#include "cal/Parser.hpp"
#include "cal/TypeChecker.hpp"
// #include "cal/CodeGenerator.hpp"
// #include "cal/bytecode/ByteCode.hpp"

int main(int argc, char** argv) {
    InitLogger();

    if (argc > 0) {
        // Run the script from args
        LogInfo("[Arguments] catched:");
        for(int i = 0; i < argc; i++) {
            LogInfo("\t[",i,"] ", argv[i]);
        }
        LogInfo("");
    }
    else {
        // Go to command line mode
    }

    // cal::CalBytecode code {};
    // code.read("test.calcls");
    // code.debugPrint();

    // return 0;

    std::string test_source = R"(
        /* 
        import std.network.socket;
        import std.io.file;
        module main; 
        */

        /*
        struct test {
            a : i32,
            b : const i32,
            c : internal i32[],
        }
        */
        
        fun main(args : string[]) {
            var c : i32;
            var a = 0;
            var b : i32 = 10;
            a = 10 + 20 * (11 + 12); 
            print(a*b);
        }

        fun test() : i32 { 
            return 10; 
        }
    )";

    cal::Lexer lex(test_source);
    lex.analyze();
    lex.debugPrint();

    cal::Parser parser(lex);
    parser.parse();
    parser.debugPrint();

    // cal::TypeChecker type_checker(parser);

    // type_checker.registerFunctionDeclear("print", { cal::ASTBase::NumType::I32 });

    // type_checker.check();
    // type_checker.debugPrint();

    // cal::CodeGenerator code_gen(parser);
    // code_gen.compile();
    // code_gen.debugPrint();
 
    // cal::CalBytecode codes = code_gen.getBytecodes();
    // codes.write("test.calcls");

    //cal::waitAnyKeyToExit();
    return 0;
}