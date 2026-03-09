/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <fstream>
#include <sstream>

#include <nbase/base/Logger.hpp>
#include <nbase/base/Assert.hpp>
#include <nbase/base/TypeTraits.hpp>

#include <string>
#include <stack>

#include <neo/ast/Decl.hpp>
#include <neo/ast/Stmts.hpp>

namespace neo
{
	struct TreeNode
	{
		String name;
		String value;
		Vector<TreeNode> nodes;
	};
	TreeNode *treeAddChild(TreeNode *c, const String &name, const String &value = "");

	class NDebugOutput
	{
	public:
		NDebugOutput();
		virtual ~NDebugOutput();

	public:
		virtual bool print() = 0;

	protected:
		struct TreeNode *m_root;
		struct TreeNode *m_current;
		std::stack<struct TreeNode *> m_last;

	public:
		void beginRoot(const StringView name);
		void endRoot();

		void beginObject(const StringView name)
		{
			auto *node = treeAddChild(m_current, name.data(), "");
			m_last.push(m_current);
			m_current = node;
		}

		void endObject()
		{
			if (m_last.empty())
				return;
			m_current = m_last.top();
			m_last.pop();
		}

		void printItem(const StringView label, const char *value)
		{
			treeAddChild(m_current, label.data(), value);
		}

		void printItem(const StringView label, const StringView value)
		{
			treeAddChild(m_current, label.data(), value.data());
		}

		void printItem(const StringView label, const String &value)
		{
			treeAddChild(m_current, label.data(), value.c_str());
		}

		template <typename T>
		void printItem(const StringView label, const T value)
		{
			treeAddChild(m_current, label.data(), neo::format("{}", value));
		}

		template <typename T, typename F>
		void printArray(const StringView label, const Vector<T> &items, F &&f)
		{
			beginObject(label);
			for (auto &item : items)
			{
				f(*this, item);
			}
			endObject();
		}

		void printArrayItem(const StringView val)
		{
			treeAddChild(m_current, val.data());
		}

		void printArrayItem(const String &val)
		{
			treeAddChild(m_current, val.c_str());
		}

		template <typename T>
		void printArrayItem(T &&val)
		{
			treeAddChild(m_current, String{std::to_string(val).data()});
		}

		template <typename... Args>
		NE_FORCE_INLINE void printItem(const StringView label, StringView fmtStr, const Args &...args)
		{
			printItem(label, neo::format(fmtStr, args...));
		}

		template <typename T>
		void printChild(const StringView label, T *child)
		{
			if (child == nullptr)
			{
				printItem(label, "<null>");
				return;
			}
			beginObject(label);

			child->debugPrint(*this);
			endObject();
		}

		template <typename T>
		void printChildren(const StringView label, const Vector<T> &items)
		{
			if (items.size() == 0)
			{
				printItem(label, "<empty>");
				return;
			}
			beginObject(label);

			auto idx = 0;
			for (auto item : items)
			{
				beginObject(neo::format("[{}]", idx));
				if constexpr (std::is_pointer_v<T>)
				{
					item->debugPrint(*this);
				}
				else
				{
					item.debugPrint(*this);
				}
				endObject();
				idx++;
			}

			endObject();
		}
	};

	template <typename T>
	void neo_ast_debug_print(NDebugOutput &o, T &&t) {}

	class NConsoleOutput final : public NDebugOutput
	{
	public:
		~NConsoleOutput() override = default;

		bool print() override;

	private:
		std::stringstream m_ss;
	};

	class NFileOutput final : public NDebugOutput
	{
	public:
		explicit NFileOutput(const StringView &path);
		~NFileOutput() override;

		bool print() override;
		void save()
		{
			m_fs.close();
		}

	private:
		std::ofstream m_fs;
	};

}
