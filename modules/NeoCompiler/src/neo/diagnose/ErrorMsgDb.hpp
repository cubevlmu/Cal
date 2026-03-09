// Created by cubevlmu on 2025/11/24.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <nbase/common.hpp>
#include <nbase/base/Format.hpp>

namespace neo {

	class ErrorMsgDb
	{
	public:
		const StringView getTranslate(const StringView key, const StringView fallback = "");

		template <typename... Args>
		const String getFormat(const StringView key, const Args&... args) {
			auto r = getTranslate(key);
			return neo::format(r, args...);
		}
		template <typename... Args>
		const String getFormat(const StringView key, const StringView fallback, const Args&... args) {
			auto r = getTranslate(key, fallback);
			return neo::format(r, args...);
		}

	private:
		ErrorMsgDb();
	    ~ErrorMsgDb();

	private:
		struct DbHandler* m_handler;
	};


	class DbManager
	{
	public:
		DbManager(const StringView transDir);
		~DbManager();

		bool loadAll();
		void close();

		ErrorMsgDb* getDb();

	public:
		HashMap<u64, struct LangDbFile*> m_db;
	};
}