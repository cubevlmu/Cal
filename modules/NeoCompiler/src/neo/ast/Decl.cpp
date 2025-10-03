// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Decl.hpp"

#include "neo/compiler/DebugOutput.hpp"
#include "Stmts.hpp"

namespace neo {

    void VarDecl::read(NSerializer *s) {
        ASTNode::read(s);
    }

    void VarDecl::write(NSerializer *s) {
        ASTNode::write(s);
        s->write(name);         // Name
        type->write(s);            // Type
        s->write(initExpr != nullptr); // isInitExprExist
        if (initExpr) {
            initExpr->write(s);
        }
    }

    void VarDecl::debugPrint(NDebugOutput &output) {
        ASTDecl::debugPrint(output);
        output.writeLine("\t   |- Name: {}", name);
        output.writeLine("\t   |- Type: ");
        if (type) type->debugPrint(output);
        output.writeLine("\t   |- InitExpr: ");
        if (initExpr) initExpr->debugPrint(output);
    }


    TopLevelDecls::~TopLevelDecls()
    {
        for (auto* ptr : decls) {
            delete ptr;
        }
        decls.clear();
    }

    ModuleDecl::~ModuleDecl()
    {
        delete children;
    }


    EnumDecl::~EnumDecl()
    {
        for (auto* ptr : children) {
            delete ptr;
        }
        children.clear();
    }


    InterfaceDecl::~InterfaceDecl()
    {
        for (auto* ptr : children) {
            delete ptr;
        }
        children.clear();
    }


    void FuncDecl::debugPrint(NDebugOutput& output) {
        ASTDecl::debugPrint(output);
        output.writeLine("\t   |- Name: {}", name);
        output.writeLine("\t   |- Args: ");
        for (auto* arg : this->args) {
            arg->debugPrint(output);
        }
        output.writeLine("\t   |- ReturnType: ");
        if (returnType) returnType->debugPrint(output);
        output.writeLine("\t   |- Body: ");
        if(funcBody) funcBody->debugPrint(output);
    }
}