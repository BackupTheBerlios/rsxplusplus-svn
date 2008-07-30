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

#include "stdinc.h"
#include "resource.h"

#include <PluginInformation.h>
#include <ClientInterface.h>
#include <UserInterface.h>

#include "Plugin.h"
#include "version.h"

bool __stdcall DllMain(HANDLE /*hInst*/, DWORD /*reason*/, LPVOID /*reserved*/) {
	return TRUE;
}

extern "C" {
	__declspec(dllexport) iPlugin* __cdecl pluginLoad() {
		Plugin::newInstance();
		return Plugin::getInstance();
	}

	__declspec(dllexport) void __cdecl pluginUnload() {
		Plugin::deleteInstance();
	}

	__declspec(dllexport) void __cdecl pluginInfo(PluginInformation& p) {
		p.pId = PLUGIN_ID;
		p.pName = L"TestAPI";
		p.pDesc = L"Plugin Description... should be short";
		p.pVersion = L"2.00";
		p.pAuthor = L"adrian_007";
		p.pApiVersion = 1211;
		p.pIconResourceId = IDB_BITMAP;
	}
}

/**
 * @file
 * $Id: main.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */