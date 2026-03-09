/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "DebugOutput.hpp"

#include <filesystem>
#include <nbase/memory/Memory.hpp>
#include <iostream>

namespace neo
{
	static void treePrintSub(
		const TreeNode *node,
		std::ostream &os,
		const String &prefix,
		bool isLast,
		const String &mid,
		const String &lastPref,
		const String &vert,
		const String &space)
	{
		os << prefix;
		os << (isLast ? lastPref : mid);
		os << node->name;
		if (!node->value.empty())
		{
			if (!node->name.empty())
				os << ": ";
			os << node->value;
		}
		os << '\n';

		String newPrefix = prefix + (isLast ? space : vert);

		for (size_t i = 0; i < node->nodes.size(); ++i)
		{
			treePrintSub(&node->nodes[i], os, newPrefix, i + 1 == node->nodes.size(),
						 mid, lastPref, vert, space);
		}
	}

	static void treePrint(TreeNode *c, std::ostream &os)
	{
		const String mid = "├── ";
		const String lastPref = "└── ";
		const String vert = "│   ";
		const String space = "    ";

		os << c->name;
		if (!c->value.empty())
			os << ": " << c->value;
		os << '\n';

		for (size_t i = 0; i < c->nodes.size(); ++i)
		{
			treePrintSub(&c->nodes[i], os, "", i + 1 == c->nodes.size(), mid, lastPref, vert, space);
		}
	}

	TreeNode *treeAddChild(TreeNode *c, const String &name, const String &value)
	{
		NE_ASSERT(c != nullptr && "TreeNode != nullptr");
		c->nodes.push_back(TreeNode{name, value});
		return &c->nodes.back();
	}

	NDebugOutput::NDebugOutput()
	{
		m_root = neo::newObject<TreeNode>();
		m_current = nullptr;
	}

	NDebugOutput::~NDebugOutput()
	{
		if (m_root)
			neo::deletePtr(m_root);
	}

	void NDebugOutput::beginRoot(const StringView name)
	{
		m_root->name = String(name);
		m_current = m_root;
	}

	void NDebugOutput::endRoot()
	{
		m_current = nullptr;
	}

	NFileOutput::NFileOutput(const StringView &path)
	{
		if (std::filesystem::exists(path.data()))
		{
			std::filesystem::remove(path.data());
		}

		m_fs.open(path.data(), std::ios::out);
		if (!m_fs.is_open())
		{
			LogError("Failed to open file");
		}
	}

	NFileOutput::~NFileOutput()
	{
		m_fs.close();
	}

	bool NConsoleOutput::print()
	{
		treePrint(m_root, std::cout);
		return true;
	}

	bool NFileOutput::print()
	{
		treePrint(m_root, m_fs);
		m_fs.flush();
		return true;
	}

}
