#include "Parser.hpp"

#include "Lexer.hpp"
#include "base/Logger.hpp"
#include "cal/ast/ASTNodes.hpp"
#include "json/json.h"

namespace cal {

#define PARSE_ERR(...) do { LogError(__VA_ARGS__);ASSERT(false); } while (false)

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
        if (m_root == nullptr)
            return;

        ASTBase::writeOutputToFile(m_root->buildOutput(), "ParserResult.json");

        LogDebug("[ParserResult] Parsed result:");
        LogDebug("\t In file ParserResult.json");
        LogDebug("\t", ASTBase::buildOutputFromJson(m_root->buildOutput()));
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


    ASTBase* Parser::parseVariableDeclear(bool isConst)
    {
        Lexer::Token tk = m_tokens[m_current];
        if (tk.tk_type != Lexer::Token::TK_IDENTIFIER)
            PARSE_ERR("variable declear need name!");
        std::string var_name = tk.tk_item;

        m_current++;
        Lexer::Token vv = m_tokens[m_current];

        ASTBase::NumType var_type = ASTBase::NumType::ERR_TP;
        ASTBase* initial_value = nullptr;

        if (vv.tk_type == Lexer::Token::TK_TYPE) {
            var_type = ASTBase::parseNumTypeText(vv.tk_item);
            if (var_type == ASTBase::NumType::ERR_TP)
                var_type = ASTBase::NumType::USER_DEFINED; // predict

            m_current++;
            if (m_tokens[m_current].tk_type != Lexer::Token::TokenType::TK_SEMICOLON) {
                initial_value = parseExpression();
            }
        }
        else if (vv.tk_type != Lexer::Token::TK_SEMICOLON) {
            var_type = ASTBase::NumType::VariableReturn; //TODO check it
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

        if (var_type == ASTBase::NumType::ERR_TP)
            PARSE_ERR("variable declear need to have a type");

        return new VariableDeclearNode(
            new IdentifierNode(IdentifierNode::Type::Variable, var_name),
            var_type,
            initial_value
        );
    }


    ASTBase* Parser::parseStatement()
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
        if (m_tokens[m_current].tk_type != Lexer::Token::TokenType::TK_LEFT_PAREN)
            PARSE_ERR("Expected '(' after function name");
        advance();


        std::vector<ASTBase*> args;
        while (!match(Lexer::Token::TK_SEMICOLON) && !match(Lexer::Token::TK_RIGHT_PAREN)) {
            // here is the smarter way dealing this
            if (peek().tk_type == Lexer::Token::TK_COMMA)
                advance();
            args.push_back(parseExpression());
        }

        if (peek().tk_type == Lexer::Token::TK_SEMICOLON)
            advance();

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
        else if (match(Lexer::Token::TK_IDENTIFIER)) {
            Lexer::Token id = m_tokens[m_current - 1];
            return new IdentifierNode(IdentifierNode::Type::Variable, id.tk_item);
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
        else if (match(Lexer::Token::TK_COMMA)) {
            //TODO
            advance();
            return nullptr;
        }
        else {
            PARSE_ERR("Unexpected token: ", peek().tk_item);
        }

        return nullptr;
    }


    ASTBase* Parser::parseFunctionDeclear() {
        Lexer::Token name = advance();
        if (name.tk_type != Lexer::Token::TK_FUNC_NAME)
            PARSE_ERR("Expected name after a function declear, but : ", name.tk_item);

        std::vector<FunctionArgNode*> arg_nodes{};

        while (m_tokens[m_current].tk_type != Lexer::Token::TK_FUNC_RETURN) {
            Lexer::Token tk = advance();
            if (tk.tk_type != Lexer::Token::TK_FUNC_ARG)
                PARSE_ERR("Expected name after function arguments declear, but: ", tk.tk_item);
            std::string arg_name = tk.tk_item;

            Lexer::Token tk_type = advance();
            if (tk_type.tk_type != Lexer::Token::TK_TYPE)
                PARSE_ERR("Function arguments should be typed, but : ", tk_type.tk_item);
            ASTBase::NumType arg_type = ASTBase::parseNumTypeText(tk_type.tk_item);

            arg_nodes.push_back(new FunctionArgNode(arg_name, arg_type));
        }

        Lexer::Token endTk = advance();
        std::string type_str = endTk.tk_item;
        if (endTk.tk_type != Lexer::Token::TK_FUNC_RETURN)
            PARSE_ERR("function should provide a return type");
        ASTBase::NumType returnType = ASTBase::parseNumTypeText(type_str);
        // Lexer::Token blockStart = advance();

        // if (blockStart.tk_type != Lexer::Token::TK_LEFT_BRACES)
        //     PARSE_ERR("Function body should be start with '{'");

        FunctionNode* node = new FunctionNode(name.tk_item, arg_nodes, returnType, (BlockNode*)parseBlock());
        return node;
    }


    ASTBase* Parser::parseBlock() {
        if (m_tokens[m_current].tk_type != Lexer::Token::TK_LEFT_BRACES)
            PARSE_ERR("Block should be start with '{");
        advance();

        std::vector<ASTBase*> statements;

        while (m_tokens[m_current].tk_type != Lexer::Token::TK_RIGHT_BRACES) {
            statements.push_back(parseStatement());
        }
        advance();

        return new BlockNode(statements);
    }


    ASTBase* Parser::parseReturn() {

        ASTBase* base = nullptr;
        while (!match(Lexer::Token::TK_SEMICOLON)) {
            // here is the smarter way dealing this
            if(base != nullptr) {
                while(!match(Lexer::Token::TK_SEMICOLON)) advance();
                break;
            }
            base = parseExpression();
        }

        if (peek().tk_type == Lexer::Token::TK_SEMICOLON)
            advance();

        return new FunctionReturnNode(base);
    }

} // namespace cal