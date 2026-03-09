/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#ifndef NE_USE_RPMALLOC
#if defined(_MSC_VER) && defined(_DEBUG)
#define NE_USE_RPMALLOC 0
#else
#define NE_USE_RPMALLOC 1
#endif
#endif
