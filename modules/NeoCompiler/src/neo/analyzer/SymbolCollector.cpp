/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "SymbolCollector.hpp"

namespace neo
{
    namespace
    {
        StringView declName(ASTDecl *decl)
        {
            switch (decl->getDeclKind())
            {
            case DeclKind::kVar:
                return static_cast<VarDecl *>(decl)->name;
            case DeclKind::kFunc:
                return static_cast<FuncDecl *>(decl)->name;
            case DeclKind::kClass:
                return static_cast<ClassDecl *>(decl)->name;
            case DeclKind::kField:
                return static_cast<FieldDecl *>(decl)->name;
            case DeclKind::kStruct:
                return static_cast<StructDecl *>(decl)->name;
            case DeclKind::kModule:
                return static_cast<ModuleDecl *>(decl)->name;
            case DeclKind::kInterface:
                return static_cast<InterfaceDecl *>(decl)->name;
            case DeclKind::kEnum:
                return static_cast<EnumDecl *>(decl)->name;
            case DeclKind::kImport:
                return static_cast<ImportDecl *>(decl)->modulePath;
            case DeclKind::kGenericParam:
                return static_cast<GenericParamDecl *>(decl)->name;
            default:
                return {};
            }
        }
    }

    SymbolCollector::SymbolCollector(SemanticContext &ctx)
        : m_ctx{ctx}
    {
    }

    Result SymbolCollector::collect(NParsedFile &file)
    {
        for (auto *node : file.Nodes)
        {
            auto result = collectNode(node);
            if (result.hasError())
            {
                return result;
            }
        }
        return Result::success();
    }

    Result SymbolCollector::collectNode(ASTNode *node)
    {
        if (node == nullptr || node->getType() != ASTType::kDeclaration)
        {
            return Result::success();
        }
        return collectDecl(static_cast<ASTDecl *>(node));
    }

    Result SymbolCollector::collectDecl(ASTDecl *decl)
    {
        if (decl == nullptr)
        {
            return Result::success();
        }

        if (auto *symbol = createDeclSymbol(decl); symbol != nullptr)
        {
            m_ctx.currentScope->declare(symbol);
            m_ctx.bindDeclSymbol(decl, symbol);
        }

        switch (decl->getDeclKind())
        {
        case DeclKind::kTopLevelDecls:
            return collectContainer(static_cast<TopLevelDecls *>(decl));
        case DeclKind::kModule:
            return collectModule(static_cast<ModuleDecl *>(decl));
        case DeclKind::kClass:
            return collectTypeMembers(static_cast<ClassDecl *>(decl));
        case DeclKind::kStruct:
            return collectTypeMembers(static_cast<StructDecl *>(decl));
        case DeclKind::kInterface:
            return collectTypeMembers(static_cast<InterfaceDecl *>(decl));
        case DeclKind::kEnum:
            return collectTypeMembers(static_cast<EnumDecl *>(decl));
        default:
            return Result::success();
        }
    }

    Result SymbolCollector::collectContainer(TopLevelDecls *decls)
    {
        if (decls == nullptr)
        {
            return Result::success();
        }

        for (auto *decl : decls->decls)
        {
            auto result = collectDecl(decl);
            if (result.hasError())
            {
                return result;
            }
        }
        return Result::success();
    }

    Result SymbolCollector::collectModule(ModuleDecl *decl)
    {
        if (decl == nullptr || decl->children == nullptr)
        {
            return Result::success();
        }

        auto *symbol = static_cast<ModuleSymbol *>(m_ctx.getDeclSymbol(decl));
        if (symbol == nullptr)
        {
            return Result::success();
        }

        symbol->moduleScope = createOwnedScope(ScopeKind::kModule, decl);
        m_ctx.pushScope(symbol->moduleScope);
        auto result = collectContainer(decl->children);
        m_ctx.popScope();
        return result;
    }

    Result SymbolCollector::collectTypeMembers(ClassDecl *decl)
    {
        auto *symbol = static_cast<ClassSymbol *>(m_ctx.getDeclSymbol(decl));
        if (decl == nullptr || symbol == nullptr)
        {
            return Result::success();
        }

        symbol->memberScope = createOwnedScope(ScopeKind::kType, decl);
        m_ctx.pushScope(symbol->memberScope);
        for (auto *field : decl->fields)
        {
            auto result = collectDecl(field);
            if (result.hasError())
            {
                return result;
            }
        }
        for (auto *var : decl->variables)
        {
            auto result = collectDecl(var);
            if (result.hasError())
            {
                return result;
            }
        }
        for (auto *func : decl->functions)
        {
            auto result = collectDecl(func);
            if (result.hasError())
            {
                return result;
            }
        }
        for (auto *ctor : decl->ctors)
        {
            auto result = collectDecl(ctor);
            if (result.hasError())
            {
                return result;
            }
        }
        if (decl->dtors != nullptr)
        {
            auto result = collectDecl(decl->dtors);
            if (result.hasError())
            {
                return result;
            }
        }
        for (auto *member : decl->errorMembers)
        {
            auto result = collectDecl(member);
            if (result.hasError())
            {
                return result;
            }
        }
        m_ctx.popScope();
        return Result::success();
    }

    Result SymbolCollector::collectTypeMembers(StructDecl *decl)
    {
        auto *symbol = static_cast<StructSymbol *>(m_ctx.getDeclSymbol(decl));
        if (decl == nullptr || symbol == nullptr)
        {
            return Result::success();
        }

        symbol->memberScope = createOwnedScope(ScopeKind::kType, decl);
        m_ctx.pushScope(symbol->memberScope);
        for (auto *var : decl->variables)
        {
            auto result = collectDecl(var);
            if (result.hasError())
            {
                return result;
            }
        }
        m_ctx.popScope();
        return Result::success();
    }

    Result SymbolCollector::collectTypeMembers(InterfaceDecl *decl)
    {
        auto *symbol = static_cast<InterfaceSymbol *>(m_ctx.getDeclSymbol(decl));
        if (decl == nullptr || symbol == nullptr)
        {
            return Result::success();
        }

        symbol->memberScope = createOwnedScope(ScopeKind::kType, decl);
        m_ctx.pushScope(symbol->memberScope);
        for (auto *func : decl->children)
        {
            auto result = collectDecl(func);
            if (result.hasError())
            {
                return result;
            }
        }
        m_ctx.popScope();
        return Result::success();
    }

    Result SymbolCollector::collectTypeMembers(EnumDecl *decl)
    {
        auto *symbol = static_cast<EnumSymbol *>(m_ctx.getDeclSymbol(decl));
        if (decl == nullptr || symbol == nullptr)
        {
            return Result::success();
        }

        symbol->memberScope = createOwnedScope(ScopeKind::kType, decl);
        m_ctx.pushScope(symbol->memberScope);
        for (auto *item : decl->children)
        {
            auto result = collectDecl(item);
            if (result.hasError())
            {
                return result;
            }
        }
        m_ctx.popScope();
        return Result::success();
    }

    Symbol *SymbolCollector::createDeclSymbol(ASTDecl *decl)
    {
        if (decl == nullptr)
        {
            return nullptr;
        }

        switch (decl->getDeclKind())
        {
        case DeclKind::kImport:
        case DeclKind::kTopLevelDecls:
            return nullptr;
        case DeclKind::kError:
            return m_ctx.ownSymbol(new ErrorSymbol("<error>"));
        case DeclKind::kVar:
            return m_ctx.ownSymbol(new VariableSymbol(SymbolKind::kVariable, declName(decl), decl));
        case DeclKind::kGenericParam:
            return m_ctx.ownSymbol(new GenericParamSymbol(declName(decl), decl));
        case DeclKind::kFunc:
            return m_ctx.ownSymbol(new FunctionSymbol(declName(decl), decl));
        case DeclKind::kClass:
            return m_ctx.ownSymbol(new ClassSymbol(declName(decl), decl));
        case DeclKind::kField:
            return m_ctx.ownSymbol(new FieldSymbol(declName(decl), decl));
        case DeclKind::kStruct:
            return m_ctx.ownSymbol(new StructSymbol(declName(decl), decl));
        case DeclKind::kModule:
            return m_ctx.ownSymbol(new ModuleSymbol(declName(decl), decl));
        case DeclKind::kInterface:
            return m_ctx.ownSymbol(new InterfaceSymbol(declName(decl), decl));
        case DeclKind::kEnum:
            return m_ctx.ownSymbol(new EnumSymbol(declName(decl), decl));
        default:
            return nullptr;
        }
    }

    Scope *SymbolCollector::createOwnedScope(ScopeKind kind, ASTNode *owner, Scope *parent)
    {
        return m_ctx.createScope(kind, owner, parent == nullptr ? m_ctx.currentScope : parent);
    }
}
