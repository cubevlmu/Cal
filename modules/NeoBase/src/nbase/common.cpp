// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "common.hpp"

namespace neo {

	static const Version s_Ver = {
		.major = 0,
		.minor = 1,
		.reverse = 1
	};

	void getNeoBaseVersion(Version& v)
	{
		memset(&v, 0, sizeof(Version));
		memcpy(&v, &s_Ver, sizeof(Version));
	}

}