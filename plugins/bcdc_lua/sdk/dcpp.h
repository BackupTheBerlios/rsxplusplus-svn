/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

// pointer type for x86/x64
#ifdef _WIN64
typedef uint64_t dcpp_ptr_t;
#else
typedef uint32_t dcpp_ptr_t;
#endif // _WIN64

#define MAKE_VER(n1, n2, n3, n4) (((uint64_t)n1 & 0xFFFF) << 48 | ((uint64_t)n2 & 0xFFFF) << 32 | ((uint64_t)n3 & 0xFFFF) << 16 | ((uint64_t)n4 & 0xFFFF))
#define VER_MAJOR(v)	(uint32_t)(((uint64_t)v & 0xFFFF000000000000) >> 48)
#define VER_MINOR(v)	(uint32_t)(((uint64_t)v & 0x0000FFFF00000000) >> 32)
#define VER_REVISION(v) (uint32_t)(((uint64_t)v & 0x00000000FFFF0000) >> 16)
#define VER_BUILD(v)	(uint32_t)(((uint64_t)v & 0x000000000000FFFF))

// PluginSDK version (include in PluginInformation)
#define SDK_VERSION MAKE_VER(2, 1, 0, 0)

// boolean logic
#define DCPP_TRUE 1
#define DCPP_FALSE 0
// default call convention
#define DCPP_CALL_CONV __stdcall

typedef struct {
	const char*			name;			// Name of the plugin
	const char*			guid;			// GUID, set if only one instance is allowed
	const char*			author;			// Author
	const char*			description;	// Short description of the plugin
	const char*			website;		// Website of the plugin
	uint64_t			version;		// Plugin's version
	uint64_t			sdkVersion;		// SDK's version used to compile plugin, set to SDK_VERSION
} dcppPluginInformation;

// function type to listen for events
typedef int (DCPP_CALL_CONV *dcppListenerFunc)(int, dcpp_ptr_t, dcpp_ptr_t);
// function type to add own caller (extend call function in dcppFunctions)
typedef dcpp_ptr_t (DCPP_CALL_CONV *dcppCallFunc)(const char*, dcpp_ptr_t, dcpp_ptr_t, dcpp_ptr_t, int*);

typedef struct {
	dcpp_ptr_t	(DCPP_CALL_CONV *call)				(const char* type, dcpp_ptr_t, dcpp_ptr_t, dcpp_ptr_t);

	int			(DCPP_CALL_CONV *addCaller)			(dcppCallFunc);
	int			(DCPP_CALL_CONV *removeCaller)		(dcppCallFunc);

	int			(DCPP_CALL_CONV *addSpeaker)		(const char*);
	int			(DCPP_CALL_CONV *removeSpeaker)		(const char*);
	int			(DCPP_CALL_CONV *isSpeaker)			(const char*);
	int			(DCPP_CALL_CONV *fireSpeaker)		(const char*, int, dcpp_ptr_t, dcpp_ptr_t);

	int			(DCPP_CALL_CONV *addListener)		(const char*, dcppListenerFunc);
	int			(DCPP_CALL_CONV *removeListener)	(const char*, dcppListenerFunc);
} dcppFunctions;

// common structures to keep data / sets of data
typedef struct {
	char*				buf;
	int					size;
} dcppBuffer;

typedef struct _dcppLM {
	void*				first;
	void*				second;
	struct _dcppLM*		next;
} dcppLinkedMap;

typedef struct _dcppLL {
	void*				element;
	struct _dcppLL*		next;
} dcppLinkedList;

//////////////////////////////////////////////////////////////////////////////////////
// Core Event Speaker
#define DCPP_EVENT_CORE "Core/"
#define DCPP_CALL_CORE "Core/"
// Core Events
#define DCPP_EVENT_CORE_LOAD			0
#define DCPP_EVENT_CORE_UNLOAD			1
#define DCPP_EVENT_CORE_SETTINGS_SAVE	2

// Core Calls
#define DCPP_CALL_CORE_SETTING_PLUG_GET		"Core/Setting/Plug/Get"
#define DCPP_CALL_CORE_SETTING_PLUG_SET		"Core/Setting/Plug/Set"
//#define DCPP_CALL_CORE_SETTING_DCPP_GET		"Core/Setting/dcpp/Get"
//#define DCPP_CALL_CORE_SETTING_DCPP_SET		"Core/Setting/dcpp/Set"

#ifdef __cplusplus
} // extern "C"
#endif

#endif //DCPP_PLUGIN_SDK

/**
 * @file
 * $Id: dcpp.h 117 2009-04-11 21:44:24Z adrian_007 $
 */
