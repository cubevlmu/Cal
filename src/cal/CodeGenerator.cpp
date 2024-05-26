#include "CodeGenerator.hpp"

#include "Parser.hpp"
#include "base/Logger.hpp"
#include "cal/ast/ASTNodes.hpp"

namespace cal {

#define CodeGen_ERR(...) \
        LogError(__VA_ARGS__); \
        ASSERT(false);


    CodeGenerator::CodeGenerator(Parser& parser)
        : m_parser(parser), m_bytecode()
    {
        ASTBase* root = parser.m_root;
        if (root == nullptr) {
            LogError("[Compiler] Faild to get ast root from parser");
            ASSERT(false);
        }
        m_ast_root = root;
    }


    void CodeGenerator::compile()
    {
        generate(m_ast_root);
    }


    void CodeGenerator::debugPrint()
    {
        m_bytecode.debugPrint();
    }


    CalBytecode CodeGenerator::getBytecodes()
    {
        return m_bytecode;
    }


    void CodeGenerator::generate(ASTBase* node)
    {
        if (auto numNode = dynamic_cast<NumberNode*>(node)) {
            generateNumber(numNode);
        }
        else if (auto opNode = dynamic_cast<OpNode*>(node)) {
            generateOperator(opNode);
        }
        else if (auto assignNode = dynamic_cast<AssignmentNode*>(node)) {
            generateAssignment(assignNode);
        }
        else if (auto varNode = dynamic_cast<IdentifierNode*>(node)) {
            if(varNode->m_type == IdentifierNode::Variable) {
                generateVariable(varNode);
            }
        }
        else if (auto funcCallNode = dynamic_cast<FunctionCallNode*>(node)) {
            generateFunctionCall(funcCallNode);
        }
        else if (auto blockNode = dynamic_cast<BlockNode*>(node)) {
            for (auto stmt : blockNode->m_statements) {
                generate(stmt);
            }
        }
        else {
            CodeGen_ERR("Unknown AST node type");
        }
    }


    void CodeGenerator::generateNumber(NumberNode* node) {
        Constants constant;
        constant.const_type = getNodeConstType(node);
        setNodeConstValue(node, constant);
        m_bytecode.addConstant(constant);

        int reg = getNextRegister();
        m_bytecode.addInstruction({ OP_LOAD_CONST, reg, m_bytecode.get().constants_size - 1, 0 });
    }


    void CodeGenerator::generateVariable(IdentifierNode* node)
    {
        if (node->m_type != IdentifierNode::Variable) {
            CodeGen_ERR("Identifier Node's type is not variable");
            return;
        }
        if (m_variableMap.find(node->m_name) == m_variableMap.end()) {
            CodeGen_ERR("Undefined variable: ", node->m_name);
            return;
        }
        int varIndex = m_variableMap[node->m_name];
        int reg = getNextRegister();
        m_bytecode.addInstruction({ OP_LOAD_VAR, reg, varIndex, 0 });
    }


    void CodeGenerator::generateOperator(OpNode* node)
    {
        generate(node->m_left);
        int leftReg = m_nextRegisterIndex - 1;
        generate(node->m_right);
        int rightReg = m_nextRegisterIndex - 1;
        int resultReg = getNextRegister();

        switch (node->m_op_type) {
        case OpNode::Plus:
            m_bytecode.addInstruction({ OP_ADD, resultReg, leftReg, rightReg });
            break;
        case OpNode::Minus:
            m_bytecode.addInstruction({ OP_SUB, resultReg, leftReg, rightReg });
            break;
        case OpNode::Multi:
            m_bytecode.addInstruction({ OP_MUL, resultReg, leftReg, rightReg });
            break;
        case OpNode::Divid:
            m_bytecode.addInstruction({ OP_DIV, resultReg, leftReg, rightReg });
            break;
        default:
            throw std::runtime_error("Unsupported operator");
        }
    }


    void CodeGenerator::generateAssignment(AssignmentNode* node)
    {
        generate(node->m_value);
        int valueReg = m_nextRegisterIndex - 1;
        int varIndex;
        if (m_variableMap.find(node->m_variable->m_name) == m_variableMap.end()) {
            varIndex = m_nextVariableIndex++;
            m_variableMap[node->m_variable->m_name] = varIndex;
        }
        else {
            varIndex = m_variableMap[node->m_variable->m_name];
        }
        m_bytecode.addInstruction({ OP_STORE, varIndex, valueReg, 0 });
    }


    void CodeGenerator::generateFunctionCall(FunctionCallNode* node)
    {
        //TODO For function calling
        // for (auto arg : node->m_arguments) {
        //     generate(arg);
        // }
        // if (node->m_functionName->m_name == "print") {
        //     int argReg = m_nextRegisterIndex - 1;
        //     // 假设 print 是挂载到虚拟机的 C 函数
        //     // 生成调用 print 的字节码
        //     // bytecode.addInstruction({ OP_CALL_C_FUNC, print_function_id, argReg, 0 });
        // } else {
        //     // 添加其他函数调用处理
        // }
    }


    int CodeGenerator::getNextRegister() {
        return m_nextRegisterIndex++;
    }


    Constants::ConstantTypes CodeGenerator::getNodeConstType(NumberNode* node)
    {
        switch (node->getNumType()) {
        case NumberNode::Int: return Constants::Int;
        case NumberNode::Float: return Constants::Float;
        case NumberNode::Double: return Constants::Double;
        case NumberNode::Long: return Constants::Long;
        default: throw std::runtime_error("Unknown number type");
        }
    }


    void CodeGenerator::setNodeConstValue(NumberNode* node, Constants& constant)
    {
        switch (node->getNumType()) {
        case NumberNode::Int:
            constant.const_int = node->getInt();
            break;
        case NumberNode::Float:
            constant.const_float = node->getFloat();
            break;
        case NumberNode::Double:
            constant.const_double = node->getDouble();
            break;
        case NumberNode::Long:
            constant.const_long = node->getLong();
            break;
        default:
            throw std::runtime_error("Unknown number type");
        }
    }


}