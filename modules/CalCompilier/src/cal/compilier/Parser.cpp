#include "Parser.hpp"

#include "Lexer.hpp"
#include "base/Logger.hpp"
#include "json/json.h"
#include "base/Utils.hpp"
#include "base/allocator/IAllocator.hpp"
#include "cal/ast/ASTAssignmentNode.hpp"
#include "cal/ast/ASTBlockNode.hpp"
#include "cal/ast/ASTFuncCallNode.hpp"
#include "cal/ast/ASTFuncDefNode.hpp"
#include "cal/ast/ASTFuncReturnNode.hpp"
#include "cal/ast/ASTIdNode.hpp"
#include "cal/ast/ASTNumberNode.hpp"
#include "cal/ast/ASTOPNode.hpp"
#include "cal/ast/ASTTypeNode.hpp"
#include "cal/ast/ASTVarDefNode.hpp"

namespace cal {

#define PARSE_ERR(...) do { LogError(__VA_ARGS__);ASSERT(false); } while (false)

    Parser::Parser(Lexer& lexer, IAllocator& alloc)
        : m_lex(lexer), m_alloc(alloc), m_tokens(alloc), m_allocator(m_alloc, "Parser allocator for ast"), m_current(0)
    {

    }


    void Parser::parse() {
        m_lex.m_tokens.copyTo(m_tokens);
        m_root = parseStatements();
    }


    void Parser::debugPrint()
    {
        if (m_root == nullptr)
            return;

        utils::writeOutputFromJson(m_root->buildOutput(), "ParserResult.json", false);

        LogDebug("[ParserResult] Parsed result:");
        LogDebug("\t In file ParserResult.json");
        LogDebug("\t", utils::buildOutputFromJson(m_root->buildOutput()));
    }


    Lexer::Token Parser::peek()
    {
        if (m_current < m_tokens.size()) {
            return m_tokens[m_current];
        }
        return Lexer::UnknownToken;
    }


    Lexer::Token Parser::advance()
    {
        if (m_current < m_tokens.size()) {
            return m_tokens[m_current++];
        }
        return Lexer::UnknownToken;
    }


    bool Parser::match(Lexer::Token::TokenType type)
    {
        if (peek().tk_type == type) {
            advance();
            return true;
        }
        return false;
    }



    ASTNodeBase* Parser::parseStatements()
    {
        ASTBlockNode* node = CAL_NEW(m_allocator, ASTBlockNode)(m_allocator);
        while (m_current < m_tokens.size()) {
            node->addNode(parseStatement());
        }
        return node;
    }


    ASTNodeBase* Parser::parseVariableDeclear(bool isConst)
    {
        Lexer::Token tk = m_tokens[m_current];
        if (tk.tk_type != Lexer::Token::TK_IDENTIFIER)
            PARSE_ERR("variable declear need name!");
        std::string var_name = tk.tk_item;

        m_current++;
        Lexer::Token vv = m_tokens[m_current];

        ASTTypeNode* var_type = nullptr;
        ASTNodeBase* initial_value = nullptr;

        if (vv.tk_type == Lexer::Token::TK_TYPE) {
            var_type = CAL_NEW(m_allocator, ASTTypeNode)(m_allocator);
            var_type->setType(vv.tk_item);

            m_current++;
            if (m_tokens[m_current].tk_type != Lexer::Token::TokenType::TK_SEMICOLON) {
                initial_value = parseExpression();
            }
        }
        else if (vv.tk_type != Lexer::Token::TK_SEMICOLON) {
            var_type = nullptr;
            initial_value = parseExpression();
        }
        else {
            PARSE_ERR("variable declear syntex error!");
        }

        if (m_tokens[m_current].tk_type != Lexer::Token::TokenType::TK_SEMICOLON) {
            PARSE_ERR("unknown syntax after variable declear");
        }
        else {
            m_current++;
        }

        // if (var_type == ASTNodeBase::NumType::ERR_TP)
        //     PARSE_ERR("variable declear need to have a type");

        ASTVarDefNode* node = CAL_NEW(m_allocator, ASTVarDefNode)(m_allocator);
        node->set(var_name, var_type, initial_value);

        return node;
    }


    ASTNodeBase* Parser::parseStatement()
    {
        if (match(Lexer::Token::TK_IDENTIFIER)) {
            return parseAssignment();
        }
        else if (match(Lexer::Token::TK_VAR)) {
            return parseVariableDeclear(false);
        }
        else if (match(Lexer::Token::TK_VAL)) {
            return parseVariableDeclear(true);
        }
        else if (match(Lexer::Token::TK_FUNC_CALL)) {
            return parseFunctionCall();
        }
        else if (match(Lexer::Token::TK_FUNC_DEF)) {
            return parseFunctionDeclear();
        }
        else if (match(Lexer::Token::TK_RETURN)) {
            return parseReturn();
        }
        else {
            PARSE_ERR("Unexpect token : ", peek().tk_item);
        }
        return nullptr;
    }


    ASTNodeBase* Parser::parseAssignment()
    {
        Lexer::Token variable = m_tokens[m_current - 1];
        if (match(Lexer::Token::TK_EQUAL)) {
            ASTNodeBase* value = parseExpression();
            if (match(Lexer::Token::TK_SEMICOLON)) {
                ASTAssignmentNode* node = CAL_NEW(m_allocator, ASTAssignmentNode)(m_allocator);
                node->set(variable.tk_item, value);
                return node;
            }
            else {
                PARSE_ERR("Expected ';' after assignment");
            }
        }
        PARSE_ERR("Expected '=' after variable");
        return nullptr;
    }


    ASTNodeBase* Parser::parseFunctionCall()
    {
        Lexer::Token function = m_tokens[m_current - 1];
        if (m_tokens[m_current].tk_type != Lexer::Token::TokenType::TK_LEFT_PAREN)
            PARSE_ERR("Expected '(' after function name");
        advance();


        ASTFuncCallNode* cnode = CAL_NEW(m_allocator, ASTFuncCallNode)(m_allocator);
        cnode->setName(function.tk_item);

        std::vector<ASTNodeBase*> args;
        while (!match(Lexer::Token::TK_SEMICOLON) && !match(Lexer::Token::TK_RIGHT_PAREN)) {
            // here is the smarter way dealing this
            if (peek().tk_type == Lexer::Token::TK_COMMA)
                advance();
            cnode->addParam(parseExpression());
        }


        return cnode;
    }


    ASTNodeBase* Parser::parseExpression()
    {
        return parseTerm();
    }


    ASTNodeBase* Parser::parseTerm()
    {
        ASTNodeBase* node = parseFactor();
        while (match(Lexer::Token::TK_PLUS)) {
            ASTNodeBase* right = parseFactor();

            ASTOPNode* op = CAL_NEW(m_allocator, ASTOPNode)(m_allocator);
            op->set(ASTOPNode::OperatorType::Add, node, right);

            node = op;
        }
        return node;
    }


    ASTNodeBase* Parser::parseFactor()
    {
        ASTNodeBase* node = parsePrimary();
        while (match(Lexer::Token::TK_MULTIPLE)) {
            ASTNodeBase* right = parsePrimary();

            ASTOPNode* op = CAL_NEW(m_allocator, ASTOPNode)(m_allocator);
            op->set(ASTOPNode::OperatorType::Multiply, node, right);

            node = op;
        }
        return node;
    }


    ASTNodeBase* Parser::parsePrimary()
    {
        if (match(Lexer::Token::TK_NUMBER)) {
            Lexer::Token number = m_tokens[m_current - 1];

            ASTNumberNode* num = CAL_NEW(m_allocator, ASTNumberNode)(m_allocator);
            num->set(number.tk_item);

            return num;
        }
        else if (match(Lexer::Token::TK_IDENTIFIER)) {
            Lexer::Token id = m_tokens[m_current - 1];

            ASTIdNode* idn = CAL_NEW(m_allocator, ASTIdNode)(m_allocator);
            idn->set(id.tk_item, ASTIdNode::Type::Variable);

            return idn;
        }
        else if (match(Lexer::Token::TK_LEFT_PAREN)) {
            ASTNodeBase* expr = parseExpression();
            if (match(Lexer::Token::TK_RIGHT_PAREN)) {
                return expr;
            }
            else {
                PARSE_ERR("Expected ')' after expression");
            }
        }
        else if (match(Lexer::Token::TK_FUNC_CALL)) {
            return parseFunctionCall();
        }
        else if (match(Lexer::Token::TK_COMMA)) {
            //TODO
            advance();
            return nullptr;
        }
        else if(match(Lexer::Token::TK_SEMICOLON)) {
            advance();
        }
        else {
            PARSE_ERR("Unexpected token: ", peek().tk_item);
        }

        return nullptr;
    }


    ASTNodeBase* Parser::parseFunctionDeclear() {
        Lexer::Token name = advance();
        if (name.tk_type != Lexer::Token::TK_FUNC_NAME)
            PARSE_ERR("Expected name after a function declear, but : ", name.tk_item);

        ASTFuncDefNode* node = CAL_NEW(m_allocator, ASTFuncDefNode)(m_allocator);


        while (m_tokens[m_current].tk_type != Lexer::Token::TK_FUNC_RETURN) {
            Lexer::Token tk = advance();
            if (tk.tk_type != Lexer::Token::TK_FUNC_ARG)
                PARSE_ERR("Expected name after function arguments declear, but: ", tk.tk_item);
            std::string arg_name = tk.tk_item;

            Lexer::Token tk_type = advance();
            if (tk_type.tk_type != Lexer::Token::TK_TYPE)
                PARSE_ERR("Function arguments should be typed, but : ", tk_type.tk_item);

            ASTTypeNode* argTypeNode = CAL_NEW(m_allocator, ASTTypeNode)(m_allocator);
            argTypeNode->setType(tk_type.tk_item);

            ASTFuncArgNode* argNode = CAL_NEW(m_allocator, ASTFuncArgNode)(m_allocator);
            argNode->set(arg_name, argTypeNode);

            node->addParams(argNode);
        }

        Lexer::Token endTk = advance();
        std::string type_str = endTk.tk_item;
        if (endTk.tk_type != Lexer::Token::TK_FUNC_RETURN)
            PARSE_ERR("function should provide a return type");

        ASTTypeNode* retTypeNode = CAL_NEW(m_allocator, ASTTypeNode)(m_allocator);
        retTypeNode->setType(type_str);

        // ASTNodeBase::NumType returnType = ASTNodeBase::parseNumTypeText(type_str);
        // Lexer::Token blockStart = advance();

        // if (blockStart.tk_type != Lexer::Token::TK_LEFT_BRACES)
        //     PARSE_ERR("Function body should be start with '{'");
        node->setName(name.tk_item);
        node->setBody((ASTBlockNode*)parseBlock());
        node->setRetType(retTypeNode);

        return node;
    }


    ASTNodeBase* Parser::parseBlock() {
        if (m_tokens[m_current].tk_type != Lexer::Token::TK_LEFT_BRACES)
            PARSE_ERR("Block should be start with '{");
        advance();

        ASTBlockNode* block = CAL_NEW(m_allocator, ASTBlockNode)(m_allocator);

        while (m_tokens[m_current].tk_type != Lexer::Token::TK_RIGHT_BRACES) {
            block->addNode(parseStatement());
        }
        advance();

        return block;
    }


    ASTNodeBase* Parser::parseReturn() {

        ASTNodeBase* base = nullptr;
        while (!match(Lexer::Token::TK_SEMICOLON)) {
            // here is the smarter way dealing this
            if (base != nullptr) {
                while (!match(Lexer::Token::TK_SEMICOLON)) advance();
                break;
            }
            base = parseExpression();
        }

        if (peek().tk_type == Lexer::Token::TK_SEMICOLON)
            advance();

        ASTFuncReturnNode* cnode = CAL_NEW(m_allocator, ASTFuncReturnNode)(m_allocator);
        cnode->set(base);

        return cnode;
    }

} // namespace cal