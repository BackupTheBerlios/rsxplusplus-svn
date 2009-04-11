/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DCPP_PLUGIN_SDK
#define DCPP_PLUGIN_SDK

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#else // assuming gcc/mingw

#include <stdint.h>

#endif // _MSC_VER

#ifdef _WIN64
typedef uint64_t dcpp_ptr_t;
#else
typedef uint32_t dcpp_ptr_t;
#endif

#define MAKE_VER(major, minor, revision, build) ((uint64_t)major & 0xFFFF) << 48 | ((uint64_t)minor & 0xFFFF) << 32 | ((uint64_t)revision & 0xFFFF) << 16 | ((uint64_t)build & 0xFFFF)
#define VER_MAJOR(v) (uint64_t)(((uint64_t)v & 0xFFFF000000000000) >> 48)
#define VER_MINOR(v) (uint64_t)(((uint64_t)v & 0x0000FFFF00000000) >> 32)
#define VER_REVISION(v) (uint64_t)(((uint64_t)v & 0x00000000FFFF0000) >> 16)
#define VER_BUILD(v) (uint64_t)(((uint64_t)v & 0x000000000000FFFF))

#define SDK_VERSION MAKE_VER(3, 0, 0, 0)

typedef struct {
	const char*		name;
	const char*		guid;
	const char*		author;
	const char*		description;
	const char*		website;
	uint64_t		version;
	uint64_t		sdkVersion;
} DCPP_PLUG_INFO;

typedef struct {
	// common memory managment functions
	void* (__cdecl *malloc)		(size_t);
	void* (__cdecl *calloc)		(size_t, size_t);
	void* (__cdecl *realloc)	(void*, size_t);
	void  (__cdecl *free)		(void*);

	void* (__cdecl *call)		(int type, void* p1, void* p2, void* p3);
	void  (__cdecl *debug)		(const char*, ...);
} DCPP_FUNCTIONS;

// nasty, but with it we can pass up to 9 params
typedef struct {
	void* p1;
	void* p2;
	void* p3;
} DCPP_DATA_PACK;

struct dcppStringMap {
	char* first;
	char* second;
	struct dcppStringMap* next;
};
typedef dcppStringMap DCPP_STRING_MAP;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //DCPP_PLUGIN_SDK