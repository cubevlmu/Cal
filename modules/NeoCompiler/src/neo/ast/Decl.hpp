/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <utility>

#include "Base.hpp"
#include "Stmts.hpp"
#include "Type.hpp"

namespace neo
{

    class ErrorDecl : public ASTDecl
    {
    public:
        ErrorDecl() : ASTDecl(DeclKind::kError) {}
        ~ErrorDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;
    };

    /// Variable declaration AST node contains non-changable and changable
    class VarDecl : public ASTDecl
    {
    public:
        VarDecl() : ASTDecl(DeclKind::kVar) {}
        VarDecl(const StringView &name, ASTTypeNode *type, ASTExpr *init = nullptr)
            : ASTDecl(DeclKind::kVar), name{name}, type{type}, initExpr{init}
        {
        }

    public:
        void debugPrint(NDebugOutput &output) override;

        void read(NSerializer *s) override;
        void write(NSerializer *s) override;

    public:
        String name;
        ASTTypeNode *type = nullptr;
        ASTExpr *initExpr = nullptr;
    };

    /// Generic parameter declaration
    class GenericParamDecl : public ASTDecl
    {
    public:
        GenericParamDecl() : ASTDecl(DeclKind::kGenericParam) {}
        GenericParamDecl(const StringView &name, ASTTypeNode *constraint = nullptr)
            : ASTDecl(DeclKind::kGenericParam), name{name}, constraint{constraint}
        {
        }
        ~GenericParamDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        String name;
        ASTTypeNode *constraint = nullptr;
    };

    /// Import declaration AST node
    class ImportDecl : public ASTDecl
    {
    public:
        ImportDecl() : ASTDecl(DeclKind::kImport) {}
        ImportDecl(const StringView &path)
            : ASTDecl(DeclKind::kImport), modulePath{path}
        {
        }
        ~ImportDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        String modulePath;
    };

    /// Function declaration AST node for scope based function declare
    class FuncDecl : public ASTDecl
    {
    public:
        FuncDecl() : ASTDecl(DeclKind::kFunc) {}
        FuncDecl(const StringView &name, Vector<GenericParamDecl *> genericParams, ASTTypeNode *retType, Vector<VarDecl *> args, class InitialStmt *init, class CompoundStmt *body = nullptr)
            : ASTDecl(DeclKind::kFunc), name{name}, genericParams{std::move(genericParams)}, args{std::move(args)}, returnType{retType}, funcBody{body}, initStmt{init}
        {
        }
        ~FuncDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        String name;
        Vector<GenericParamDecl *> genericParams;
        Vector<VarDecl *> args;
        ASTTypeNode *returnType = nullptr;
        CompoundStmt *funcBody = nullptr;
        class InitialStmt *initStmt = nullptr;
    };

    /// Field declaration AST node
    class FieldDecl : public ASTDecl
    {
    public:
        FieldDecl()
            : ASTDecl(DeclKind::kField) {}
        FieldDecl(const StringView &name, ASTTypeNode *type, ASTExpr *init = nullptr)
            : ASTDecl(DeclKind::kField), name{name}, type{type}, init{init}, setFuncName{}, getFuncName{}
        {
        }
        ~FieldDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Field's name
        String name;
        /// Setter function of current field.
        String setFuncName;
        /// Getter function of current field.
        String getFuncName;
        /// Type hint of current field. Nullptr for auto deducate from value.
        ASTTypeNode *type = nullptr;
        /// Initial value of current field. Nullptr for non defined initial statement.
        ASTExpr *init = nullptr;
    };

    /// Class declaration AST node
    class ClassDecl : public ASTDecl
    {
    public:
        ClassDecl() : ASTDecl(DeclKind::kClass) {}
        ClassDecl(const StringView &name, Vector<ASTTypeNode *> baseClasses)
            : ASTDecl(DeclKind::kClass), name{name}, baseClasses{std::move(baseClasses)}
        {
        }
        ~ClassDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Class's name
        String name;
        /// Super classes of current class
        Vector<ASTTypeNode *> baseClasses;
        /// Sub-classes of current class
        Vector<ASTDecl *> subDataTypes;
        /// Fields of current class
        Vector<FieldDecl *> fields;
        /// Variables of current class
        Vector<VarDecl *> variables;
        /// Functions of current class
        Vector<FuncDecl *> functions;
        /// Invalid members recovered from class body
        Vector<ASTDecl *> errorMembers;
        /// Contructors of current class.
        Vector<FuncDecl *> ctors;
        /// Destructor of current class. Nullptr for non defined.
        FuncDecl *dtors = nullptr;
    };

    /// Struct declaration AST node
    class StructDecl : public ASTDecl
    {
    public:
        StructDecl() : ASTDecl(DeclKind::kStruct) {}
        StructDecl(const StringView &name)
            : ASTDecl(DeclKind::kStruct), name{name}
        {
        }
        ~StructDecl() override = default;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Struct's name
        String name;
        /// Variables in struct's body
        Vector<VarDecl *> variables;
    };

    /// Interface declaration AST node
    class InterfaceDecl : public ASTDecl
    {
    public:
        InterfaceDecl() : ASTDecl(DeclKind::kInterface) {}
        InterfaceDecl(const StringView &name)
            : ASTDecl(DeclKind::kInterface), name{name}
        {
        }
        ~InterfaceDecl() override;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Name of current interface class
        String name;
        /// Super classes of this interface class
        Vector<FuncDecl *> children;
        /// Super interfaces of current interface
        Vector<ASTTypeNode *> baseInterfaces;
    };

    /// Enum declaration AST node
    class EnumDecl : public ASTDecl
    {
    public:
        EnumDecl() : ASTDecl(DeclKind::kEnum) {}
        EnumDecl(const StringView &name)
            : ASTDecl(DeclKind::kEnum), name{name}, baseType{nullptr}
        {
        }
        ~EnumDecl() override;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Name of current Enum class
        String name;
        /// Enum items of current enum. Presentate as variable AST node to support enum value assign.
        Vector<VarDecl *> children;
        /// Basic type inherit for current enum class. Nullptr for i32 based enum.
        ASTTypeNode *baseType = nullptr;
    };

    /// Module declaration AST node
    class ModuleDecl : public ASTDecl
    {
    public:
        ModuleDecl() : ASTDecl(DeclKind::kModule) {}
        ModuleDecl(const StringView &name)
            : ASTDecl(DeclKind::kModule), name{name}
        {
        }
        ~ModuleDecl() override;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Name of current module.
        String name;
        /// Body of current module. Nullptr for empty module scope or placeholder module declaration.
        class TopLevelDecls *children = nullptr;
    };

    /// Top level declaration for TopLevel Scripts
    class TopLevelDecls : public ASTDecl
    {
    public:
        TopLevelDecls() : ASTDecl(DeclKind::kTopLevelDecls) {}
        ~TopLevelDecls() override;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        /// Children declarations for current container.
        Vector<ASTDecl *> decls;
    };
}
