#pragma once

#include <string>
#include <vector>

namespace cal {

    class ASTBase {
    public:
        static const char* NumTypeChar[];

        enum NodeType {
            NODE_NUMBER,
            NODE_OP, NODE_ASSIGNMENT, NODE_FUNC_CALL,
            NODE_INDENTIFIER, NODE_BLOCK
        };

        enum NumType {
            Float, Double, Int, Long,
            NotANumber, ERR_TP,
            FunctionReturn, VariableReturn,
            VOID
        };

        virtual std::string toString() const = 0;
        virtual NodeType getType() const = 0;
        virtual NumType getNumType() const = 0;
    };


    class NumberNode : public ASTBase
    {
    public:

        NumberNode(const std::string& numberStr);
        virtual std::string toString() const;
        virtual NodeType getType() const { return NODE_NUMBER; }

        int getInt() { return i; }
        float getFloat() { return f; }
        double getDouble() { return d; }
        long getLong() { return l; }
        NumType getType() { return number_type; }
        virtual NumType getNumType() const { return number_type; }

    private:
        static NumType checkType(const std::string& num_str);

    private:
        union {
            int i;
            float f;
            double d;
            long l;
        };

        NumberNode::NumType number_type;
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
        virtual std::string toString() const;
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
        virtual std::string toString() const;
        virtual NodeType getType() const { return NODE_INDENTIFIER; }
        virtual NumType getNumType() const;

        std::string m_name;
        Type m_type;
    };


    // For assign to variable
    class AssignmentNode : public ASTBase
    {
    public:
        AssignmentNode(IdentifierNode* variable, ASTBase* value);
        virtual std::string toString() const;
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
        virtual std::string toString() const;
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
        virtual std::string toString() const;
        virtual NodeType getType() const { return NODE_BLOCK; }
        virtual NumType getNumType() const { return NotANumber; }

        std::vector<ASTBase*> m_statements;
    };


}