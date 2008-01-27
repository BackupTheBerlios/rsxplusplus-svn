/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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

#include "stdafx.h"
#include "PluginAPI.h"
#include "Plugin.h"
#include "version.h"

#pragma comment(lib, "RSXPlusPlus.lib")

BOOL APIENTRY DllMain(HANDLE /*hInst*/, DWORD /*reason*/, LPVOID /*reserved*/) {
	return TRUE;
}

//obligatory functions!
extern "C" {
	__declspec(dllexport) int __cdecl pluginAPI() {
		//API version of client/user/plugin interfaces
		return 1000; //1.0.0.0
	}

	__declspec(dllexport) int __cdecl pluginId() {
		return PLUGIN_ID;
	}

	__declspec(dllexport) iPlugin* __cdecl pluginLoad() {
		Plugin::newInstance();
		return Plugin::getInstance();
	}

	__declspec(dllexport) void __cdecl pluginUnload() {
		Plugin::deleteInstance();
	}
}

/**
 * @file
 * $Id: main.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */