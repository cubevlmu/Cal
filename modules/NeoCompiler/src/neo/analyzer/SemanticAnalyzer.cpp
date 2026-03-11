/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "SemanticAnalyzer.hpp"

#include "neo/ast/Decl.hpp"
#include "neo/ast/Exprs.hpp"
#include "neo/ast/Stmts.hpp"
#include "neo/ast/Type.hpp"

namespace neo
{
    SemanticAnalyzer::SemanticAnalyzer(DiagnosticCollector &diagnostics)
        : m_ctx{&diagnostics}, m_collector{m_ctx}
    {
    }

    Result SemanticAnalyzer::analyze(NParsedFile &file)
    {
        auto collectResult = m_collector.collect(file);
        if (collectResult.hasError())
        {
            return collectResult;
        }

        auto typeResult = resolveTopLevelTypes(file);
        if (typeResult.hasError())
        {
            return typeResult;
        }

        auto bodyResult = analyzeBodies(file);
        if (bodyResult.hasError())
        {
            return bodyResult;
        }

        auto contractResult = checkContracts(file);
        if (contractResult.hasError())
        {
            return contractResult;
        }

        return Result::success();
    }

    Result SemanticAnalyzer::resolveTopLevelTypes(NParsedFile &file)
    {
        for (auto *node : file.Nodes)
        {
            if (node == nullptr || node->getType() != ASTType::kDeclaration)
            {
                continue;
            }
            auto result = analyzeDecl(static_cast<ASTDecl *>(node));
            if (result.hasError())
            {
                return result;
            }
        }
        return Result::success();
    }

    Result SemanticAnalyzer::analyzeBodies(NParsedFile &file)
    {
        for (auto *node : file.Nodes)
        {
            if (node == nullptr || node->getType() != ASTType::kDeclaration)
            {
                continue;
            }
            auto result = analyzeDecl(static_cast<ASTDecl *>(node));
            if (result.hasError())
            {
                return result;
            }
        }
        return Result::success();
    }

    Result SemanticAnalyzer::checkContracts(NParsedFile &)
    {
        // Reserved for override/interface/generic-constraint checks.
        return Result::success();
    }

    Result SemanticAnalyzer::analyzeDecl(ASTDecl *decl)
    {
        if (decl == nullptr)
        {
            return Result::success();
        }

        switch (decl->getDeclKind())
        {
        case DeclKind::kTopLevelDecls:
            for (auto *child : static_cast<TopLevelDecls *>(decl)->decls)
            {
                auto result = analyzeDecl(child);
                if (result.hasError())
                {
                    return result;
                }
            }
            break;
        case DeclKind::kModule:
            if (auto *children = static_cast<ModuleDecl *>(decl)->children; children != nullptr)
            {
                auto result = analyzeDecl(children);
                if (result.hasError())
                {
                    return result;
                }
            }
            break;
        case DeclKind::kClass:
            for (auto *base : static_cast<ClassDecl *>(decl)->baseClasses)
            {
                auto result = resolveTypeNode(base);
                if (result.hasError())
                {
                    return result;
                }
            }
            break;
        case DeclKind::kFunc:
        {
            auto *func = static_cast<FuncDecl *>(decl);
            for (auto *param : func->args)
            {
                if (param->type != nullptr)
                {
                    auto result = resolveTypeNode(param->type);
                    if (result.hasError())
                    {
                        return result;
                    }
                }
            }
            if (func->returnType != nullptr)
            {
                auto result = resolveTypeNode(func->returnType);
                if (result.hasError())
                {
                    return result;
                }
            }
            break;
        }
        case DeclKind::kVar:
        {
            auto *var = static_cast<VarDecl *>(decl);
            if (var->type != nullptr)
            {
                auto result = resolveTypeNode(var->type);
                if (result.hasError())
                {
                    return result;
                }
            }
            break;
        }
        case DeclKind::kField:
        {
            auto *field = static_cast<FieldDecl *>(decl);
            if (field->type != nullptr)
            {
                auto result = resolveTypeNode(field->type);
                if (result.hasError())
                {
                    return result;
                }
            }
            break;
        }
        case DeclKind::kStruct:
        case DeclKind::kInterface:
        case DeclKind::kEnum:
        case DeclKind::kGenericParam:
        case DeclKind::kImport:
        case DeclKind::kError:
        default:
            break;
        }

        return Result::success();
    }

    Result SemanticAnalyzer::analyzeStmt(ASTStmt *stmt)
    {
        (void)stmt;
        return Result::success();
    }

    Result SemanticAnalyzer::analyzeExpr(ASTExpr *expr)
    {
        (void)expr;
        return Result::success();
    }

    Result SemanticAnalyzer::resolveTypeNode(ASTTypeNode *typeNode)
    {
        if (typeNode == nullptr)
        {
            return Result::success();
        }

        if (m_ctx.getResolvedType(typeNode) == nullptr)
        {
            m_ctx.bindResolvedType(typeNode, m_ctx.builtins.typeError);
        }
        return Result::success();
    }
}
