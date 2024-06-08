#include "TypeChecker.hpp"

#include "Parser.hpp"
#include "base/Logger.hpp"
#include "base/Utils.hpp"
#include "cal/ast/ASTNodes.hpp"
#include <initializer_list>
#include <utility>

namespace cal {

#define TypeCheck_ERR(...) \
        do { \
            LogError(__VA_ARGS__); \
            ASSERT(false); \
        } while(false)

#define TypeCheck_DEAL_WITH_FUNC_CALL(node, type) \
        if(type == ASTBase::FunctionReturn) { \
            auto* func_call_node = static_cast<FunctionCallNode*>(node); \
            type = getFunctionReturnType(func_call_node->m_functionName->m_name); \
            if(type == ASTBase::ERR_TP) { \
                TypeCheck_ERR("A function calling not found in symbol tree"); \
            } \
        }

#define TypeCheck_DEAL_WITH_VARIBALE(node, type) \
        if (node->getType() == ASTBase::NODE_INDENTIFIER) { \
            auto* id_node = static_cast<IdentifierNode*>(node); \
            if (type == ASTBase::NumType::VariableReturn) { \
                auto& name = id_node->m_name; \
                type = getVariableType(name); \
                if (type == ASTBase::ERR_TP) \
                    TypeCheck_ERR("Argument is not defined or type error"); \
            } \
        }

#define TypeCheck_UNPACK_OP(node, out_tp) \
        if(node->getType() == ASTBase::NodeType::NODE_OP) { \
            out_tp = checkOperator(static_cast<OpNode*>(node)); \
        }

#define TypeCheck_CHECK_PARAMS_RESULT_ERR(params_result, code_block) \
       if(paramTypes.size() == 1) { \
            if(paramTypes.at(0) == ASTBase::ERR_TP) \
            { \
                code_block \
            } \
        } 

#define TypeCheck_UPDATE_OP_NUM_TP(node, tp) \
        if(node->getType() == ASTBase::NODE_OP) { \
            auto* op_node = static_cast<OpNode*>(node); \
            op_node->m_type_checked = tp; \
        }

#define TypeCheck_TYPE_2_STR(type) ASTBase::NumTypeChar[type]


    TypeChecker::TypeChecker(Parser& parser)
        : m_parser(parser)
    {
        if (parser.m_root == nullptr) {
            TypeCheck_ERR("Ast root is nullptr");
        }
        m_ast_root = parser.m_root;
    }


    void TypeChecker::check()
    {
        if (m_ast_root->getType() != ASTBase::NODE_BLOCK)
        {
            TypeCheck_ERR("Ast root type is not code block, failed");
            return;
        }
        BlockNode* node = static_cast<BlockNode*>(m_ast_root);
        for (auto* node : node->m_statements)
        {
            switch (node->getType())
            {
            case ASTBase::NodeType::NODE_FUNC:
            {
                checkFunctionDeclear(static_cast<FunctionNode*>(node));
                break;
            }
            case ASTBase::NodeType::NODE_VAR_DECLEAR:
                checkVariableDeclear(static_cast<VariableDeclearNode*>(node));
                break;
            default:
                TypeCheck_ERR("You can only write function or variable declear at global scope");
                return;
            }
        }
        //m_variables.clear();
        //m_functionDeclars.clear();

        checkAST(m_ast_root);
    }


    void TypeChecker::debugPrint()
    {
        LogDebug("[TypeChecker] Rigstered variables : ", m_variables.size());
        for (auto& var : m_variables) {
            LogDebug("\t [Name] ", var.first, " [Type] ", ASTBase::NumTypeChar[var.second]);
        }

        LogDebug("[TypeChecker] Registered functions : ", m_functionDeclars.size());
        for (auto& func : m_functionDeclars) {
            BeginAppender();
            for (auto& ttp : func.second.m_argument_types)
            {
                AppenderAppend(TypeCheck_TYPE_2_STR(ttp));
                AppenderAppend(" ");
            }

            LogDebug("\t [Name] ", func.first, " [Arguments] { ", EndAppender(), "} [Type] ", TypeCheck_TYPE_2_STR(func.second.m_return_type));
        }
    }
    

    void TypeChecker::registerFunctionDeclear(const std::string& name, std::initializer_list<ASTBase::NumType> arguments, ASTBase::NumType return_type)
    {
        registerFunctionDeclear(name, std::vector<ASTBase::NumType>(arguments.begin(), arguments.end()), return_type);   
    }
    
    
    void TypeChecker::registerFunctionDeclear(const std::string& name, std::vector<ASTBase::NumType> arguments, ASTBase::NumType return_type)
    {
        auto it = m_functionDeclars.find(name);
        if (it != m_functionDeclars.end()) {
            TypeCheck_ERR("Function ", name, " already registed");
            return;
        }
        FuncDeclear dec{
            .m_argument_types = arguments,
            .m_return_type = return_type
        };
        m_functionDeclars.insert(std::make_pair(name, dec));
        LogDebug("[TypeChecker] Registed function declear ", name);
    }


    void TypeChecker::checkAST(ASTBase* node)
    {
        if (auto numNode = dynamic_cast<NumberNode*>(node)) {
            checkNumber(numNode);
        }
        else if (auto opNode = dynamic_cast<OpNode*>(node)) {
            checkOperator(opNode);
        }
        else if (auto varDeclearNode = dynamic_cast<VariableDeclearNode*>(node)) {
            checkVariableDeclear(varDeclearNode);
        }
        else if (auto assignNode = dynamic_cast<AssignmentNode*>(node)) {
            checkAssignment(assignNode);
        }
        else if (auto funcCallNode = dynamic_cast<FunctionCallNode*>(node)) {
            checkFunctionCall(funcCallNode);
        }
        else if (auto blockNode = dynamic_cast<BlockNode*>(node)) {
            for (auto stmt : blockNode->m_statements) {
                checkAST(stmt);
            }
        }
        else if (auto idNode = dynamic_cast<IdentifierNode*>(node)) {
            checkIdNode(idNode);
        }
        else {
            TypeCheck_ERR("Unknown AST node type");
        }
    }


    void TypeChecker::checkNumber(NumberNode* node)
    {
        // 检查数字节点是否正确
        if (node->getType() == NumberNode::NumType::NotANumber) {
            TypeCheck_ERR("Invalid number literal");
        }
    }


    ASTBase::NumType TypeChecker::checkOperator(OpNode* node)
    {
        checkAST(node->m_left);
        checkAST(node->m_right);

        NumberNode::NumType left_tp = node->m_left->getNumType();
        // unpack op node to final num type
        TypeCheck_UNPACK_OP(node->m_left, left_tp);
        // deal with variable
        TypeCheck_DEAL_WITH_VARIBALE(node->m_left, left_tp);
        // deal with function calling
        TypeCheck_DEAL_WITH_FUNC_CALL(node->m_left, left_tp);


        NumberNode::NumType right_tp = node->m_right->getNumType();
        // unpack op node to final num type
        TypeCheck_UNPACK_OP(node->m_right, right_tp);
        // deal with variable
        TypeCheck_DEAL_WITH_VARIBALE(node->m_right, right_tp);
        // deal with function calling
        TypeCheck_DEAL_WITH_FUNC_CALL(node->m_right, right_tp);


        if (left_tp == NumberNode::NotANumber || right_tp == NumberNode::NotANumber) {
            TypeCheck_ERR("Invalid types for binary operation");
            return ASTBase::ERR_TP;
        }
        if (left_tp != right_tp) {
            TypeCheck_ERR("Type mismatch in binary operation.");
            return ASTBase::ERR_TP;
        }

        TypeCheck_UPDATE_OP_NUM_TP(node, left_tp);

        return left_tp;
    }


    void TypeChecker::checkVariableDeclear(VariableDeclearNode* node)
    {
        if (node->m_initial_value == nullptr && node->m_variable_type == ASTBase::NumType::VariableReturn)
            TypeCheck_ERR("Expected type declear after the variable name");

        if (node->m_initial_value != nullptr) {
            checkAST(node->m_initial_value);
        }

        if (node->m_variable_type == ASTBase::NumType::VariableReturn) {
            node->m_variable_type = node->m_initial_value->getNumType();
            if (node->m_variable_type == ASTBase::NumType::ERR_TP)
                TypeCheck_ERR("Unexpected initial value for this variable. Initial value's type is invalid");
        }
        else if (node->m_variable_type == ASTBase::NumType::USER_DEFINED) {
            //TODO
        }

        if (hasThisVariable(node->m_name->m_name))
            TypeCheck_ERR("Unexpected declear before this variable declear (samename)");

        m_variables.insert(std::make_pair(node->m_name->m_name, node->m_variable_type));
    }


    void TypeChecker::checkAssignment(AssignmentNode* node)
    {
        checkAST(node->m_value);

        ASTBase::NumType value_tp = node->m_value->getNumType();
        // deal with function calling
        TypeCheck_DEAL_WITH_FUNC_CALL(node->m_value, value_tp);

        // 假设变量类型已知，可以在变量表中查找
        IdentifierNode* id_node = node->m_variable;
        ASTBase::NumType var_tp = getVariableType(id_node->m_name);

        if (value_tp != var_tp) {
            TypeCheck_ERR("Type mismatch in assignment. ", TypeCheck_TYPE_2_STR(value_tp), "!=", TypeCheck_TYPE_2_STR(var_tp));
        }
    }


    void TypeChecker::checkFunctionCall(FunctionCallNode* node)
    {
        for (auto& arg : node->m_arguments) {
            checkAST(arg);
        }

        // check function declear types
        bool is_found;
        std::vector<ASTBase::NumType> paramTypes = getFunctionParamTypes(node->m_functionName->m_name);
        const auto& args = node->m_arguments;

        TypeCheck_CHECK_PARAMS_RESULT_ERR(paramTypes, {
            TypeCheck_ERR("Function not declear -> ", node->m_functionName->m_name);
            return;
            });

        if (args.size() != paramTypes.size()) {
            TypeCheck_ERR("Argument count mismatch in function call");
        }

        for (size_t i = 0; i < args.size(); ++i) {
            //FunctionReturn
            auto current_num_type = args[i]->getNumType();
            // deal with the variable
            TypeCheck_DEAL_WITH_VARIBALE(args[i], current_num_type);
            // deal with function calling
            TypeCheck_DEAL_WITH_FUNC_CALL(args[i], current_num_type);


            // check each argument's type with declear types
            if (current_num_type != paramTypes[i]) {
                TypeCheck_ERR("Argument type mismatch in function call");
            }
        }
    }


    void TypeChecker::checkIdNode(IdentifierNode* node)
    {
        switch (node->m_type) {
        case IdentifierNode::FuncCallArg:
        case IdentifierNode::Variable:
            if (!hasThisVariable(node->m_name))
                TypeCheck_ERR("Variable not found -> ", node->m_name);
            break;
        case IdentifierNode::Function:
            if (!hasThisFunction(node->m_name))
                TypeCheck_ERR("Variable not found -> ", node->m_name);
            break;
        }
    }


    void TypeChecker::checkFunctionDeclear(FunctionNode* node) {
        if (hasThisFunction(node->m_name))
            TypeCheck_ERR("Function has been declear before this declear");

        std::vector<ASTBase::NumType> m_arg_types;
        for (auto* arg : node->m_args) {
            if (arg->m_type == ASTBase::ERR_TP)
                TypeCheck_ERR("Function argument's type can't be invalid");
            else if (arg->m_type == ASTBase::USER_DEFINED) {
                //TODO check user defined
            }
            m_arg_types.push_back(arg->getNumType());
        }


        if (node->m_returnType == ASTBase::ERR_TP)
            TypeCheck_ERR("Function return type can't be invalid");
        else if (node->m_returnType == ASTBase::USER_DEFINED) {
            //TODO check user defined
        }

        checkAST(node->m_func_content);

        registerFunctionDeclear(node->m_name, m_arg_types, node->m_returnType);
    }


    ASTBase::NumType TypeChecker::getVariableType(const std::string& name)
    {
        auto it = m_variables.find(name);
        if (it != m_variables.end()) {
            return it->second;
        }
        else {
            // LogDebug("[TypeChecker] Insert variable ", name, " Type ", TypeCheck_TYPE_2_STR(maybe_type));
            // m_variables.insert(std::make_pair(name, maybe_type));
            // return maybe_type;
            TypeCheck_ERR("Can't resolve this variable : ", name);
            return ASTBase::ERR_TP;
        }
    }


    std::vector<ASTBase::NumType> TypeChecker::getFunctionParamTypes(const std::string& name)
    {
        auto it = m_functionDeclars.find(name);
        if (it != m_functionDeclars.end()) {
            return it->second.m_argument_types;
        }
        else {
            return { ASTBase::ERR_TP };
        }
    }


    ASTBase::NumType TypeChecker::getFunctionReturnType(const std::string& name)
    {
        auto it = m_functionDeclars.find(name);
        if (it != m_functionDeclars.end()) {
            return it->second.m_return_type;
        }
        else {
            return ASTBase::NumType::ERR_TP;
        }
    }


    bool TypeChecker::hasThisVariable(const std::string& name)
    {
        auto it = m_variables.find(name);
        return it != m_variables.end();
    }


    bool TypeChecker::hasThisFunction(const std::string& name)
    {
        auto it = m_functionDeclars.find(name);
        return it != m_functionDeclars.end();
    }


}