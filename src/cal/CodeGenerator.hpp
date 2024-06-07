#pragma once

// #include "cal/ast/ASTNodes.hpp"
// #include "cal/bytecode/ByteCode.hpp"
// #include <unordered_map>

// namespace cal {

//     class Parser;

//     class CodeGenerator
//     {
//     public:
//         CodeGenerator(Parser& parser);

//         void compile();
//         void debugPrint();
//         CalBytecode getBytecodes();

//     private:
//         void generate(ASTBase* ast);
        
//         void generateNumber(NumberNode* node);
//         void generateVariable(IdentifierNode* node);
//         void generateOperator(OpNode* node);
//         void generateAssignment(AssignmentNode* node);
//         void generateFunctionCall(FunctionCallNode* node);

//         int getNextRegister();
//         Constants::ConstantTypes getNodeConstType(NumberNode* node);
//         void setNodeConstValue(NumberNode* node, Constants& constant);

//     private:
//         Parser& m_parser;

//         ASTBase* m_ast_root;
//         // CalBytecode m_bytecode;

//         std::unordered_map<std::string, ASTBase::NumType> m_variableMap;
//     };
// }