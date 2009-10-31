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

#define MAKE_VER(major, minor, revision, build) (((uint64_t)major & 0xFFFF) << 48 | ((uint64_t)minor & 0xFFFF) << 32 | ((uint64_t)revision & 0xFFFF) << 16 | ((uint64_t)build & 0xFFFF))
#define VER_MAJOR(v)	(uint32_t)(((uint64_t)v & 0xFFFF000000000000) >> 48)
#define VER_MINOR(v)	(uint32_t)(((uint64_t)v & 0x0000FFFF00000000) >> 32)
#define VER_REVISION(v) (uint32_t)(((uint64_t)v & 0x00000000FFFF0000) >> 16)
#define VER_BUILD(v)	(uint32_t)(((uint64_t)v & 0x000000000000FFFF))

#define SDK_VERSION MAKE_VER(2, 0, 0, 0)

typedef int (__stdcall *dcppListenerFunc)(dcpp_ptr_t, dcpp_ptr_t, dcpp_ptr_t);

typedef struct {
	const char*			name;			// Name of the plugin
	const char*			guid;			// unique GUID
	const char*			author;			// Author
	const char*			description;	// Short description of the plugin
	const char*			website;		// Website of the plugin
	uint64_t			version;		// Plugin's version
	uint64_t			sdkVersion;		// SDK's version used to compile plugin, set to SDK_VERSION
} dcppPluginInformation;

typedef struct {
	void*		(__cdecl *malloc)			(size_t);
	void*		(__cdecl *calloc)			(size_t, size_t);
	void*		(__cdecl *realloc)			(void*, size_t);
	void		(__cdecl *free)				(void*);

	dcpp_ptr_t	(__stdcall *call)			(int type, dcpp_ptr_t, dcpp_ptr_t, dcpp_ptr_t);

	void*		(__stdcall *addListener)	(int, dcppListenerFunc, dcpp_ptr_t);
	void		(__stdcall *removeListener)	(void*);
} dcppFunctions;


typedef struct {
	const char*			message;
	dcpp_ptr_t			hub;
} dcppHubMessage;

typedef struct {
	const char*			message;
	dcpp_ptr_t			hub;
	dcpp_ptr_t			from;
	dcpp_ptr_t			to;
	dcpp_ptr_t			replyTo;
	int8_t				thirdPerson;
} dcppPrivateMessageIn;

typedef struct {
	const char*			message;
	dcpp_ptr_t			hub;
	dcpp_ptr_t			to;
	int8_t				thirdPerson;
} dcppPrivateMessageOut;

typedef struct {
	const char*			line;
	dcpp_ptr_t			connection;
	uint16_t			flags;
} dcppConnectionMessage;

typedef struct {
	char*				buf;
	int					size;
} dcppBuffer;

typedef struct {
	dcpp_ptr_t			hub;
	const char*			url;
} dcppHubRedirect;

typedef struct _dcppLinkedMap {
	void*				first;
	void*				second;
	struct _dcppLinkedMap* next;
} dcppLinkedMap;

typedef struct _dcppLinkedList {
	void*				element;
	struct _dcppLinkedList* next;
} dcppLinkedList;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //DCPP_PLUGIN_SDK

/**
 * @file
 * $Id: dcpp.h 117 2009-04-11 21:44:24Z adrian_007 $
 */
