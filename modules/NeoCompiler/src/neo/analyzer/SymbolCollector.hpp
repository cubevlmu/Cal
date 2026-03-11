/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "SemanticContext.hpp"
#include "neo/ast/Decl.hpp"
#include "neo/compiler/ParsedFile.hpp"

namespace neo
{
    class SymbolCollector
    {
    public:
        explicit SymbolCollector(SemanticContext &ctx);

    public:
        Result collect(NParsedFile &file);

    private:
        Result collectNode(ASTNode *node);
        Result collectDecl(ASTDecl *decl);
        Result collectContainer(TopLevelDecls *decls);
        Result collectModule(ModuleDecl *decl);
        Result collectTypeMembers(ClassDecl *decl);
        Result collectTypeMembers(StructDecl *decl);
        Result collectTypeMembers(InterfaceDecl *decl);
        Result collectTypeMembers(EnumDecl *decl);

        Symbol *createDeclSymbol(ASTDecl *decl);
        Scope *createOwnedScope(ScopeKind kind, ASTNode *owner, Scope *parent = nullptr);

    private:
        SemanticContext &m_ctx;
    };
}
