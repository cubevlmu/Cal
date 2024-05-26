#include "Parser.hpp"

#include "Lexer.hpp"
#include "base/Logger.hpp"
#include "cal/ast/ASTNodes.hpp"

namespace cal {

#define PARSE_ERR(...) \
            LogError(__VA_ARGS__); \
            ASSERT(false)

    Parser::Parser(Lexer& lexer)
        : m_lex(lexer)
    {

    }


    void Parser::parse() {
        m_tokens = m_lex.m_tokens;
        m_root = parseStatements();
    }


    void Parser::debugPrint()
    {
        if(m_root == nullptr) 
            return;

        LogDebug("[ParserResult] Parsed result:");
        LogDebug("\t", m_root->toString());
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



    ASTBase* Parser::parseStatements()
    {
        // 解析多个语句
        std::vector<ASTBase*> statements;
        while (m_current < m_tokens.size()) {
            statements.push_back(parseStatement());
        }
        return new BlockNode(statements);
    }


    ASTBase* Parser::parseStatement()
    {
        if (match(Lexer::Token::TK_VARIABLE)) {
            return parseAssignment();
        }
        else if (match(Lexer::Token::TK_FUNC_CALL)) {
            return parseFunctionCall();
        }
        else {
            PARSE_ERR("Unexpect token : ", peek().tk_item);
        }
        return nullptr;
    }


    ASTBase* Parser::parseAssignment()
    {
        Lexer::Token variable = m_tokens[m_current - 1];
        if (match(Lexer::Token::TK_EQUAL)) {
            ASTBase* value = parseExpression();
            if (match(Lexer::Token::TK_SEMICOLON)) {
                return new AssignmentNode(new IdentifierNode(IdentifierNode::Type::Variable, variable.tk_item), value);
            }
            else {
                PARSE_ERR("Expected ';' after assignment");
            }
        }
        PARSE_ERR("Expected '=' after variable");
        return nullptr;
    }


    ASTBase* Parser::parseFunctionCall()
    {
        Lexer::Token function = m_tokens[m_current - 1];
        std::vector<ASTBase*> args;
        while (!match(Lexer::Token::TK_SEMICOLON)) {
            args.push_back(new IdentifierNode(IdentifierNode::FuncCallArg, peek().tk_item));
            advance();
        }
        return new FunctionCallNode(new IdentifierNode(IdentifierNode::Function, function.tk_item), args);
    }


    ASTBase* Parser::parseExpression()
    {
        return parseTerm();
    }


    ASTBase* Parser::parseTerm()
    {
        ASTBase* node = parseFactor();
        while (match(Lexer::Token::TK_PLUS)) {
            ASTBase* right = parseFactor();
            node = new OpNode(OpNode::Plus, node, right);
        }
        return node;
    }


    ASTBase* Parser::parseFactor()
    {
        ASTBase* node = parsePrimary();
        while (match(Lexer::Token::TK_MULTIPLE)) {
            ASTBase* right = parsePrimary();
            node = new OpNode(OpNode::Multi, node, right);
        }
        return node;
    }


    ASTBase* Parser::parsePrimary()
    {
        if (match(Lexer::Token::TK_NUMBER)) {
            Lexer::Token number = m_tokens[m_current - 1];
            return new NumberNode(number.tk_item);
        }
        else if (match(Lexer::Token::TK_LEFT_PAREN)) {
            ASTBase* expr = parseExpression();
            if (match(Lexer::Token::TK_RIGHT_PAREN)) {
                return expr;
            }
            else {
                PARSE_ERR("Expected ')' after expression");
            }
        }
        else {
            PARSE_ERR("Unexpected token: ", peek().tk_item);
        }

        return nullptr;
    }
}