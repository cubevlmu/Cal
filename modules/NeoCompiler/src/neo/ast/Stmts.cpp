/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Stmts.hpp"

#include "neo/compiler/DebugOutput.hpp"

namespace neo
{

	CompoundStmt::CompoundStmt()
		: ASTStmt(StmtKind::kCompound)
	{
	}

	void CompoundStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("CompoundStmt");

		out.printChildren("statements", statements);

		out.endObject();
	}

	void IfStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("IfStmt");

		out.printChild("IfCondition", ifExpr);
		out.printChild("defaultBranch", defaultBranch);
		out.printChildren("elseIfBranches", elseIfBranches);

		if (elseBranch)
		{
			out.printChild("elseBranch", elseBranch);
		}
		else
		{
			out.printItem("elseBranch", "");
		}

		out.endObject();
	}

	void WhileStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("WhileStmt");

		out.printChild("condition", condition);
		out.printChild("body", body);

		out.endObject();
	}

	void ForStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ForStmt");

		out.printChild("declaration", declVar);
		out.printChild("cond", cond);
		out.printChild("update", update);
		out.printChild("body", body);

		out.endObject();
	}

	void ForeachStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ForeachStmt");

		out.printChild("declearation", declearation);
		out.printChild("object", object);

		out.endObject();
	}

	void ReturnStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ReturnStmt");

		out.printChild("ret", ret);

		out.endObject();
	}

	void BreakStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("BreakStmt");
		out.endObject();
	}

	void ContinueStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ContinueStmt");
		out.endObject();
	}

	void CatchClause::debugPrint(NDebugOutput &out)
	{
		ASTNode::debugPrint(out);
		out.beginObject("CatchClause");

		out.printItem("varName", varName);
        out.printChild("errorType", errorType);
		out.printChild("handlerBody", handlerBody);

		out.endObject();
	}

	void TryStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("TryStmt");

		out.printChild("body", body);
		out.printChildren("handlers", handlers);
		out.printChild("finallyBody", finallyBody);

		out.endObject();
	}

	void ThrowStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ThrowStmt");

		out.printChild("expr", expr);

		out.endObject();
	}

	void ImportStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ImportStmt");

		out.printItem("module", moduleName);

		out.endObject();
	}

	void DeclStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("DeclStmt");

		out.printChild("decl", decl);

		out.endObject();
	}

	void ExprStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ExprStmt");
		out.printChild("expr", expr);
		out.endObject();
	}

	void ErrorStmt::debugPrint(NDebugOutput &out)
	{
		ASTStmt::debugPrint(out);
		out.beginObject("ErrorStmt");
		out.endObject();
	}

    void InitialStmt::debugPrint(NDebugOutput &out)
    {
		ASTNode::debugPrint(out);
		out.beginObject("InitialStmt");

		out.printChildren("initializers", initialStmts);

		out.endObject();
    }
}
