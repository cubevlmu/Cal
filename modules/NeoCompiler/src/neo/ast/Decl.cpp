/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Decl.hpp"

#include "neo/compiler/DebugOutput.hpp"

namespace neo
{
	void ErrorDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("ErrorDecl");
		out.endObject();
	}

	void VarDecl::read(NSerializer *s)
	{
		ASTNode::read(s);
	}

	void VarDecl::write(NSerializer *s)
	{
		ASTNode::write(s);
		s->write(name);				   // Name
		type->write(s);				   // Type
		s->write(initExpr != nullptr); // isInitExprExist
		if (initExpr)
		{
			initExpr->write(s);
		}
	}

	void VarDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("VarDecl");

		out.printItem("name", name);
		out.printChild("type", type);
		out.printChild("init", initExpr);

		out.endObject();
	}

	void GenericParamDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("GenericParamDecl");

		out.printItem("name", name);
		out.printChild("constraint", constraint);

		out.endObject();
	}

	void ImportDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("ImportDecl");

		out.printItem("path", modulePath);

		out.endObject();
	}

	TopLevelDecls::~TopLevelDecls()
	{
		for (auto *ptr : decls)
		{
			delete ptr;
		}
		decls.clear();
	}

	void TopLevelDecls::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("TopLevelDecls");

		out.printChildren("children", decls);

		out.endObject();
	}

	ModuleDecl::~ModuleDecl()
	{
		delete children;
	}

	void ModuleDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("ModuleDecl");

		out.printItem("name", name);
		out.printChild("body", children);

		out.endObject();
	}

	EnumDecl::~EnumDecl()
	{
		for (auto *ptr : children)
		{
			delete ptr;
		}
		children.clear();
	}

	void EnumDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("EnumDecl");

		out.printItem("name", name);
		out.printChildren("items", children);
		out.printChild("baseType", baseType);

		out.endObject();
	}

	InterfaceDecl::~InterfaceDecl()
	{
		for (auto *ptr : children)
		{
			delete ptr;
		}
		children.clear();
	}

	void InterfaceDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("InterfaceDecl");

		out.printItem("name", name);
		out.printChildren("children", children);
		out.printChildren("bases", baseInterfaces);

		out.endObject();
	}

	void FuncDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("FuncDecl");

		out.printItem("name", name);
		out.printChildren("genericParams", genericParams);
		out.printChild("return", returnType);
		out.printChildren("args", args);
		out.printChild("body", funcBody);
		out.printChild("baseInitializers", initStmt);

		out.endObject();
	}

	void FieldDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("FieldDecl");

		out.printItem("name", name);
		out.printChild("type", type);
		out.printChild("init", init);

		out.endObject();
	}

	void ClassDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("ClassDecl");

		out.printItem("name", name);
		out.printChildren("bases", baseClasses);
		out.printChildren("subTypes", subDataTypes);
		out.printChildren("fields", fields);
		out.printChildren("variables", variables);
		out.printChildren("functions", functions);
		out.printChildren("errorMembers", errorMembers);
		out.printChildren("constructors", ctors);
		out.printChild("destructor", dtors);

		out.endObject();
	}

	void StructDecl::debugPrint(NDebugOutput &out)
	{
		ASTDecl::debugPrint(out);
		out.beginObject("StructDecl");

		out.printItem("name", name);
		out.printChildren("variables", variables);

		out.endObject();
	}

}
