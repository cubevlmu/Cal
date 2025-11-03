// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <utility>

#include "Base.hpp"
#include "Type.hpp"

namespace neo {

	/// Variable declaration AST node contains non-changable and changable
    class VarDecl : public ASTDecl
    {
    public:
        VarDecl() : ASTDecl(DeclKind::kVar) {}
        VarDecl(const std::string_view& name, ASTTypeNode* type, ASTExpr* init = nullptr)
            : ASTDecl(DeclKind::kVar)
            , name{ name }
            , type{ type }
            , initExpr{ init }
        {
        }

    public:
        void debugPrint(NDebugOutput& output) override;

        void read(NSerializer* s) override;
        void write(NSerializer* s) override;

    public:
        std::string name;
        ASTTypeNode* type = nullptr;
        ASTExpr* initExpr = nullptr;
    };


	/// Function declaration AST node for scope based function declare
    class FuncDecl : public ASTDecl
    {
    public:
        FuncDecl() : ASTDecl(DeclKind::kFunc) {}
        FuncDecl(const std::string_view& name, ASTTypeNode* retType, std::vector<VarDecl*> args, class CompoundStmt* body = nullptr)
            : ASTDecl(DeclKind::kFunc)
            , name{ name }
            , args{std::move( args )}
            , returnType{ retType }
            , funcBody{ body }
        {
        }
        ~FuncDecl() override = default;

    public:
        void debugPrint(NDebugOutput& output) override;

    public:
        std::string name;
        std::vector<VarDecl*> args;
        ASTTypeNode* returnType = nullptr;
        CompoundStmt* funcBody = nullptr;
    };


	/// Field declaration AST node
    class FieldDecl : public ASTDecl
    {
    public:
        FieldDecl()
            : ASTDecl(DeclKind::kField) {}
        FieldDecl(const std::string_view& name, ASTTypeNode* type, ASTExpr* init = nullptr)
            : ASTDecl(DeclKind::kField)
            , name{ name }
            , type{ type }
            , init{ init }
            , setFuncName{}
            , getFuncName{}
        {
        }
        ~FieldDecl() override = default;

    public:
		/// Field's name
        std::string name;
		/// Setter function of current field.
        std::string setFuncName;
		/// Getter function of current field.
        std::string getFuncName;
		/// Type hint of current field. Nullptr for auto deducate from value.
		ASTTypeNode* type = nullptr;
		/// Initial value of current field. Nullptr for non defined initial statement.
        ASTExpr* init = nullptr;
    };


	/// Class declaration AST node
    class ClassDecl : public ASTDecl
    {
    public:
        ClassDecl() : ASTDecl(DeclKind::kClass) {}
        ClassDecl(const std::string_view& name, std::vector<ASTTypeNode*> baseClasses)
            : ASTDecl(DeclKind::kClass)
            , name{ name }
            , baseClasses{std::move( baseClasses )}
        {
        }
        ~ClassDecl() override = default;

    public:
		/// Class's name
        std::string name;
		/// Super classes of current class
        std::vector<ASTTypeNode*> baseClasses;
		/// Sub-classes of current class
        std::vector<ASTDecl*> subDataTypes;
		/// Fields of current class
        std::vector<FieldDecl*> fields;
		/// Variables of current class
        std::vector<VarDecl*> variables;
		/// Functions of current class
        std::vector<FuncDecl*> functions;
		/// Contructors of current class.
        std::vector<FuncDecl*> ctors;
		/// Destructor of current class. Nullptr for non defined.
        FuncDecl* dtors = nullptr;
    };


	/// Struct declaration AST node
    class StructDecl : public ASTDecl
    {
    public:
        StructDecl() : ASTDecl(DeclKind::kStruct) {}
        StructDecl(const std::string_view& name)
            : ASTDecl(DeclKind::kStruct)
            , name{ name }
        {
        }
        ~StructDecl() override = default;

	public:
		/// Struct's name
        std::string name;
		/// Variables in struct's body
        std::vector<VarDecl*> variables;
    };


	/// Interface declaration AST node
    class InterfaceDecl : public ASTDecl
    {
    public:
        InterfaceDecl() : ASTDecl(DeclKind::kInterface) {}
        InterfaceDecl(const std::string_view& name)
            : ASTDecl(DeclKind::kInterface)
            , name{ name }
        {
        }
        ~InterfaceDecl() override;

    public:
		/// Name of current interface class
        std::string name;
		/// Super classes of this interface class
        std::vector<FuncDecl*> children;
	    /// Super interfaces of current interface
	    std::vector<ASTTypeNode*> baseInterfaces;
    };


	/// Enum declaration AST node
    class EnumDecl : public ASTDecl
    {
    public:
        EnumDecl() : ASTDecl(DeclKind::kEnum) {}
        EnumDecl(const std::string_view& name)
            : ASTDecl(DeclKind::kEnum)
            , name{ name }
            , baseType{nullptr}
        {
        }
        ~EnumDecl() override;

    public:
		/// Name of current Enum class
        std::string name;
		/// Enum items of current enum. Presentate as variable AST node to support enum value assign.
        std::vector<VarDecl*> children;
		/// Basic type inherit for current enum class. Nullptr for i32 based enum.
        ASTTypeNode* baseType = nullptr;
    };


	/// Module declaration AST node
    class ModuleDecl : public ASTDecl
    {
    public:
        ModuleDecl() : ASTDecl(DeclKind::kModule) {}
        ModuleDecl(const std::string_view& name)
            : ASTDecl(DeclKind::kModule)
            , name{ name }
        {
        }
        ~ModuleDecl() override;

    public:
		/// Name of current module.
        std::string name;
		/// Body of current module. Nullptr for empty module scope or placeholder module declaration.
        class TopLevelDecls* children = nullptr;
    };


	/// Top level declaration for TopLevel Scripts
    class TopLevelDecls : public ASTDecl 
    {
    public:
        TopLevelDecls() : ASTDecl(DeclKind::kTopLevelDecls) {}
        ~TopLevelDecls() override;
    
    public:
		/// Children declarations for current container.
        std::vector<ASTDecl*> decls;
    };
}