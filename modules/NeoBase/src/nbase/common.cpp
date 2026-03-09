/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

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