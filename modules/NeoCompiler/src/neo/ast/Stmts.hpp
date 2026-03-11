/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "Base.hpp"
#include "Type.hpp"

#include <nbase/types/String.hpp>
#include <initializer_list>
#include <utility>

namespace neo
{

    /// A collection contains multiple statement nodes
    class CompoundStmt : public ASTStmt
    {
    public:
        CompoundStmt();
        CompoundStmt(Vector<ASTStmt *> stmts)
            : ASTStmt(StmtKind::kCompound), statements{std::move(stmts)}
        {
        }
        CompoundStmt(std::initializer_list<ASTStmt *> stmts)
            : ASTStmt(StmtKind::kCompound), statements{stmts}
        {
        }
        ~CompoundStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        Vector<ASTStmt *> statements;
    };

    /// If-else statement AST node
    class IfStmt final : public ASTStmt
    {
    public:
        IfStmt()
            : ASTStmt(StmtKind::kIf), ifExpr{nullptr}, defaultBranch{nullptr}, elseBranch{nullptr}, elseIfBranches{}
        {
        }
        IfStmt(ASTExpr *ifExpr, ASTStmt *defaultBranch, ASTStmt *elseBranch = nullptr)
            : ASTStmt(StmtKind::kIf), ifExpr{ifExpr}, defaultBranch{defaultBranch}, elseBranch{elseBranch}, elseIfBranches{}
        {
        }
        ~IfStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTExpr *ifExpr;
        ASTStmt *defaultBranch;
        Vector<ASTStmt *> elseIfBranches;
        ASTStmt *elseBranch = nullptr;
    };

    /// while-loop statement AST node
    class WhileStmt : public ASTStmt
    {
    public:
        WhileStmt() : ASTStmt(StmtKind::kWhile) {}
        WhileStmt(ASTExpr *cond, ASTStmt *body)
            : ASTStmt(StmtKind::kWhile), condition{cond}, body{body}
        {
        }
        ~WhileStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTExpr *condition;
        ASTStmt *body;
    };

    /// for-loop statement AST node
    class ForStmt : public ASTStmt
    {
    public:
        ForStmt() : ASTStmt(StmtKind::kFor) {}
        ForStmt(class VarDecl *decl, ASTExpr *cond, ASTExpr *update, ASTStmt *body)
            : ASTStmt(StmtKind::kFor), declVar{decl}, cond{cond}, update{update}, body{body}
        {
        }
        ~ForStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        VarDecl *declVar;
        ASTExpr *cond;
        ASTExpr *update;
        ASTStmt *body;
    };

    /// foreach-loop statement AST node
    class ForeachStmt : public ASTStmt
    {
    public:
        ForeachStmt(ASTStmt *decl, ASTExpr *object)
            : ASTStmt(StmtKind::kForeach), declearation{decl}, object{object}
        {
        }
        ~ForeachStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTStmt *declearation;
        ASTExpr *object;
    };

    /// Scope return statement AST node
    class ReturnStmt : public ASTStmt
    {
    public:
        ReturnStmt() : ASTStmt(StmtKind::kReturn), ret{nullptr} {}
        ReturnStmt(ASTExpr *expr)
            : ASTStmt(StmtKind::kReturn), ret{expr}
        {
        }
        ~ReturnStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTExpr *ret;
    };

    /// Loop break or scope break statement AST node
    class BreakStmt : public ASTStmt
    {
    public:
        BreakStmt() : ASTStmt(StmtKind::kBreak) {}
        ~BreakStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;
    };

    /// Catch clause for exception handling
    class CatchClause : public ASTNode
    {
    public:
        CatchClause() : ASTNode(ASTType::kStatement) {}
        CatchClause(String varName, ASTTypeNode *type, ASTStmt *body)
            : ASTNode(ASTType::kStatement), varName{std::move(varName)}, errorType{type}, handlerBody{body}
        {
        }

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        String varName;
        ASTTypeNode *errorType = nullptr;
        ASTStmt *handlerBody;
    };

    /// Try-catch block statement
    class TryStmt : public ASTStmt
    {
    public:
        TryStmt() : ASTStmt(StmtKind::kTry) {}
        TryStmt(ASTStmt *body)
            : ASTStmt(StmtKind::kTry), body{body}, handlers{}
        {
        }

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTStmt *body;
        Vector<CatchClause *> handlers;
        ASTStmt *finallyBody = nullptr;
    };

    /// Exception emit statement (throw)
    class ThrowStmt : public ASTStmt
    {
    public:
        ThrowStmt() : ASTStmt(StmtKind::kThrow) {}
        ThrowStmt(ASTExpr *expr)
            : ASTStmt(StmtKind::kThrow), expr{expr}
        {
        }

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTExpr *expr;
    };

    /// Loop continue or jump statement AST node
    class ContinueStmt : public ASTStmt
    {
    public:
        ContinueStmt() : ASTStmt(StmtKind::kContinue) {}
        ~ContinueStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;
    };

    /// Module import statement AST node for module system
    class ImportStmt : public ASTStmt
    {
    public:
        ImportStmt(neo::NString name)
            : ASTStmt(StmtKind::kImport), moduleName{std::move(name)}
        {
        }
        ~ImportStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        const neo::NString moduleName;
    };

    /// Declaration container for scope declaration statement
    class DeclStmt : public ASTStmt
    {
    public:
        DeclStmt(ASTDecl *decl)
            : ASTStmt(StmtKind::kDecl), decl{decl}
        {
        }
        ~DeclStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTDecl *decl;
    };

    /// Expression container for scope expression statement
    class ExprStmt : public ASTStmt
    {
    public:
        ExprStmt(ASTExpr *expr)
            : ASTStmt(StmtKind::kExpr), expr{expr}
        {
        }
        ~ExprStmt() override = default;

    public:
        void debugPrint(NDebugOutput &out) override;

    public:
        ASTExpr *expr;
    };

    class ErrorStmt : public ASTStmt
    {
    public:
        ErrorStmt() : ASTStmt(StmtKind::kError) {}
        ~ErrorStmt() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;
    };

    class InitialStmt : public ASTNode
    {
    public:
        InitialStmt() : ASTNode(ASTType::kInitializer) {}
        ~InitialStmt() override = default;

    public:
        void debugPrint(NDebugOutput& out) override;

    public:
        Vector<ASTExpr*> initialStmts;
    };
}
