#pragma once

#include "cal/ast/ASTNodes.hpp"
#include <initializer_list>
#include <unordered_map>

namespace cal {

    class Parser;

    class TypeChecker 
    {
    public:
        TypeChecker(Parser& parser);

        void check();
        void debugPrint();

        void registerFunctionDeclear(const std::string& name, std::initializer_list<ASTBase::NumType> arguments, ASTBase::NumType return_type = ASTBase::NumType::VOID);
        void registerFunctionDeclear(const std::string& name, std::vector<ASTBase::NumType> arguments, ASTBase::NumType return_type = ASTBase::NumType::VOID);

    private:
        void checkAST(ASTBase* node);
        
        void checkNumber(NumberNode* node);
        ASTBase::NumType checkOperator(OpNode* node);
        void checkVariableDeclear(VariableDeclearNode* node);
        void checkAssignment(AssignmentNode* node);
        void checkFunctionCall(FunctionCallNode* node);
        void checkIdNode(IdentifierNode* node);
        void checkFunctionDeclear(FunctionNode* node);

        ASTBase::NumType getVariableType(const std::string& name);
        std::vector<ASTBase::NumType> getFunctionParamTypes(const std::string& name); 
        ASTBase::NumType getFunctionReturnType(const std::string& name); 
        
        bool hasThisVariable(const std::string& name);
        bool hasThisFunction(const std::string& name);

    private:
        struct FuncDeclear {
            std::vector<ASTBase::NumType> m_argument_types;
            ASTBase::NumType m_return_type;
        };

        struct UserDefineType {
            //TODO  
        };

        Parser& m_parser;
        ASTBase* m_ast_root;

        std::unordered_map<std::string, ASTBase::NumType> m_variables;
        std::unordered_map<std::string, FuncDeclear> m_functionDeclars;

        //TODO
        std::unordered_map<std::string, UserDefineType> m_userdefined_types;
    };
}