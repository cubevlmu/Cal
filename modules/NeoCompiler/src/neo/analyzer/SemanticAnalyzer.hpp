/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "SemanticContext.hpp"
#include "SymbolCollector.hpp"
#include "neo/compiler/ParsedFile.hpp"

namespace neo
{
    class ASTStmt;
    class ASTExpr;
    class ASTTypeNode;

    class SemanticAnalyzer
    {
    public:
        explicit SemanticAnalyzer(DiagnosticCollector &diagnostics);

    public:
        Result analyze(NParsedFile &file);
        SemanticContext &context()
        {
            return m_ctx;
        }

    private:
        Result resolveTopLevelTypes(NParsedFile &file);
        Result analyzeBodies(NParsedFile &file);
        Result checkContracts(NParsedFile &file);

        Result analyzeDecl(ASTDecl *decl);
        Result analyzeStmt(ASTStmt *stmt);
        Result analyzeExpr(ASTExpr *expr);
        Result resolveTypeNode(ASTTypeNode *typeNode);

    private:
        SemanticContext m_ctx;
        SymbolCollector m_collector;
    };
}
