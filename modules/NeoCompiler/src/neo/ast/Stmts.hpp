// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include "Base.hpp"

#include <initializer_list>
#include <utility>
#include <vector>

namespace neo {

	/// A collection contains multiple statement nodes
    class CompoundStmt : public ASTStmt
    {
    public:
        CompoundStmt();
        CompoundStmt(std::vector<ASTStmt*> stmts)
            : ASTStmt(StmtKind::kCompound)
            , statements{std::move( stmts )}
        {
        }
        CompoundStmt(std::initializer_list<ASTStmt*> stmts)
            : ASTStmt(StmtKind::kCompound)
            , statements{ stmts }
        {
        }
        ~CompoundStmt() override = default;

    public:

    public:
        std::vector<ASTStmt*> statements;
    };


	/// If-else statement AST node
    class IfStmt final : public ASTStmt
    {
    public:
        IfStmt(ASTExpr* ifExpr, ASTStmt* defaultBranch, ASTStmt* elseBranch = nullptr)
            : ASTStmt(StmtKind::kIf)
            , ifExpr{ ifExpr }
            , defaultBranch{ defaultBranch }
            , elseBranch{ elseBranch }
        {
        }
        ~IfStmt() override = default;

    public:
        ASTExpr* ifExpr;
        ASTStmt* defaultBranch;
        ASTStmt* elseBranch = nullptr;;
    };


	/// while-loop statement AST node
    class WhileStmt : public ASTStmt
    {
    public:
        WhileStmt(ASTExpr* cond, ASTStmt* body)
            : ASTStmt(StmtKind::kWhile)
            , condition{ cond }
            , body{ body }
        {
        }
        ~WhileStmt() override = default;

    public:
        ASTExpr* condition;
        ASTStmt* body;
    };


	/// for-loop statement AST node
    class ForStmt : public ASTStmt
    {
    public:
        ForStmt(ASTStmt* decl, ASTExpr* cond, ASTExpr* update, ASTStmt* body)
            : ASTStmt(StmtKind::kFor)
            , declVar{ decl }
            , cond{ cond }
            , update{ update }
            , forBody{ body }
        {
        }
        ~ForStmt() override = default;

    public:
        ASTStmt* declVar;
        ASTExpr* cond;
        ASTExpr* update;
        ASTStmt* forBody;
    };


	/// foreach-loop statement AST node
    class ForeachStmt : public ASTStmt
    {
    public:
        ForeachStmt(ASTStmt* decl, ASTExpr* object)
            : ASTStmt(StmtKind::kForeach)
            , declearation{ decl }
            , object{ object }
        {
        }
        ~ForeachStmt() override = default;

    public:
        ASTStmt* declearation;
        ASTExpr* object;
    };


	/// Scope return statement AST node
    class ReturnStmt : public ASTStmt
    {
    public:
        ReturnStmt(ASTExpr* expr)
            : ASTStmt(StmtKind::kReturn)
            , ret{ expr }
        {
        }
        ~ReturnStmt() override = default;

    public:
        ASTExpr* ret;
    };


	/// Loop break or scope break statement AST node
    class BreakStmt : public ASTStmt
    {
    public:
        BreakStmt() : ASTStmt(StmtKind::kBreak) {}
        ~BreakStmt() override = default;
    };


	/// Loop continue or jump statement AST node
    class ContinueStmt : public ASTStmt
    {
    public:
        ContinueStmt() : ASTStmt(StmtKind::kContinue) {}
        ~ContinueStmt() override = default;
    };


	/// Module import statement AST node for module system
    class ImportStmt : public ASTStmt 
    {
    public:
        ImportStmt(std::string name)
            : ASTStmt(StmtKind::kImport)
            , moduleName {std::move(name)}
        {}
        ~ImportStmt() override = default;

    public:
        const std::string moduleName;
    };


	/// Declaration container for scope declaration statement
    class DeclStmt : public ASTStmt
    {
    public:
        DeclStmt(ASTDecl* decl)
            : ASTStmt(StmtKind::kDecl)
            , declType {decl}
        {}
        ~DeclStmt() override = default;

    public:
        ASTDecl* declType;
    };


	/// Expression container for scope expression statement
    class ExprStmt : public ASTStmt
    {
    public:
        ExprStmt(ASTExpr* expr)
            : ASTStmt(StmtKind::kExpr)
            , expr {expr}
        {}
        ~ExprStmt() override = default;

    public:
        ASTExpr* expr;
    };
}