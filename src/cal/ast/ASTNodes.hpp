#pragma once

#include <string>
#include <vector>
#include "globals.hpp"

namespace Json {
class Value;
}

namespace cal {

    class ASTBase {
    public:
        static const char* NumTypeChar[];

        enum NodeType {
            NODE_NUMBER,
            NODE_OP, NODE_ASSIGNMENT, NODE_FUNC_CALL,
            NODE_INDENTIFIER, NODE_BLOCK,
            NODE_VAR_DECLEAR, NODE_TYPE, 
            NODE_FUNC, NODE_FUNC_ARG, NODE_FUNC_RETURN
        };

        enum NumType {
            Float, Double, 
            I32, U32, I64, U64, U8, I8, U16, I16, BOOL,
            USER_DEFINED,
            NotANumber, ERR_TP,
            FunctionReturn, VariableReturn,
            VOID
        };

        virtual Json::Value buildOutput() const = 0;
        virtual NodeType getType() const = 0;
        virtual NumType getNumType() const = 0;

        static NumType parseNumTypeText(const std::string& type);
        static std::string buildOutputFromJson(const Json::Value& val);
        static bool writeOutputToFile(const Json::Value& val, const std::string& file);
    };


    class NumberNode : public ASTBase
    {
    public:
        NumberNode(const std::string& numberStr);
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const { return NODE_NUMBER; }

        int getInt() { return i; }
        float getFloat() { return f; }
        double getDouble() { return d; }
        long getLong() { return l; }
        NumType getType() { return number_type; }
        virtual NumType getNumType() const { return number_type; }

        union {
            float f;
            double d;
            i64 l;
            u64 u64;
            i32 i;
            u32 u32;
            i16 i16;
            u16 u16;
            u8 u8;
            i8 i8;
        };

        NumberNode::NumType number_type;

    private:
        static NumType checkType(const std::string& num_str);
    };


    class OpNode : public ASTBase
    {
    public:
        enum OpType {
            Multi, // *
            Divid, // /
            Plus,  // +
            Minus // -
        };

        OpType m_op_type;
        ASTBase* m_left;
        ASTBase* m_right;
        NumType m_type_checked = NumType::ERR_TP;

        OpNode(OpType type, ASTBase* left, ASTBase* right);
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const { return NODE_OP; }
        virtual NumType getNumType() const { return m_type_checked; }
    };


    // For variable name or function name
    class IdentifierNode : public ASTBase
    {
    public:
        enum Type {
            Variable,
            Function,
            FuncCallArg
        };

        IdentifierNode(Type type, const std::string& name);
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const { return NODE_INDENTIFIER; }
        virtual NumType getNumType() const;

        std::string m_name;
        Type m_type;
    };


    class TypeNode : public ASTBase 
    {
    public:
        TypeNode(const std::string& node_str);

        virtual Json::Value buildOutput() const override;
        virtual NodeType getType() const override;
        virtual NumType getNumType() const override;

        NumType m_type;
        std::string m_raw_type;
        bool m_isArray;
    };


    class VariableDeclearNode : public ASTBase 
    {
    public:
        VariableDeclearNode(IdentifierNode* name_node, NumType var_type, ASTBase* initial_value);
        
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const;
        virtual NumType getNumType() const;

        IdentifierNode* m_name;
        NumType m_variable_type;
        ASTBase* m_initial_value;
    };


    // For assign to variable
    class AssignmentNode : public ASTBase
    {
    public:
        AssignmentNode(IdentifierNode* variable, ASTBase* value);
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const { return NODE_ASSIGNMENT; }
        virtual NumType getNumType() const { return NotANumber; }

        IdentifierNode* m_variable;
        ASTBase* m_value;
    };


    // For function calling
    class FunctionCallNode : public ASTBase
    {
    public:
        FunctionCallNode(IdentifierNode* functionName, std::vector<ASTBase*> arguments);
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const { return NODE_FUNC_CALL; }
        virtual NumType getNumType() const { return FunctionReturn; }

        IdentifierNode* m_functionName;
        std::vector<ASTBase*> m_arguments;
    };


    // Function blocks
    class BlockNode : public ASTBase
    {
    public:
        BlockNode(std::vector<ASTBase*> statements);
        virtual Json::Value buildOutput() const;
        virtual NodeType getType() const { return NODE_BLOCK; }
        virtual NumType getNumType() const { return NotANumber; }

        std::vector<ASTBase*> m_statements;
    };


    class FunctionArgNode : public ASTBase 
    {
    public:
        FunctionArgNode(const std::string& name, NumType type);

        virtual Json::Value buildOutput() const override;
        virtual NumType getNumType() const override;
        virtual NodeType getType() const override 
        { return NODE_FUNC_ARG; }

        std::string m_name;
        NumType m_type;
    };


    // Function declear
    class FunctionNode : public ASTBase 
    {
    public:
        FunctionNode(const std::string& name, std::vector<FunctionArgNode*> args, NumType returnType, BlockNode* content);
        
        virtual Json::Value buildOutput() const;
        virtual NumType getNumType() const;
        virtual NodeType getType() const { return NODE_FUNC; }

        std::vector<FunctionArgNode*> m_args;
        std::string m_name;
        NumType m_returnType;
        BlockNode* m_func_content;
    };


    class FunctionReturnNode : public ASTBase 
    {
    public:
        FunctionReturnNode(ASTBase* returnValue);

        virtual Json::Value buildOutput() const;
        virtual NumType getNumType() const { return m_returnValue->getNumType(); }
        virtual NodeType getType() const { return NODE_FUNC_RETURN; }

        ASTBase* m_returnValue;
        bool m_voidReturn;
    };
}