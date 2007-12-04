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
#include "Plugin.h"

HINSTANCE hInstance = NULL;

BOOL APIENTRY DllMain(HANDLE hInst, DWORD reason, LPVOID /*reserved*/) {
	switch(reason) {
		case DLL_PROCESS_ATTACH:
			hInstance = (HINSTANCE)hInst;
			break;
		case DLL_PROCESS_DETACH:
			hInstance = NULL;
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

//init and uinit functions
extern "C" {
	//this must be unique, if not, host won't load plug-in
	int EXPORT pluginId() {
		return PLUGIN_ID;
	}
	//user api version
	int EXPORT pluginAPIVersion() {
		return PLUGIN_USED_API_VERSION;
	}
	//pointer to main class
	PluginAPI EXPORT *pluginLoad() {
		Plugin::newInstance();
		//need it to load plugin icon
		Plugin::getInstance()->setInstance(hInstance);
		return Plugin::getInstance();
	}
	//called on host exit/plugin reload
	void EXPORT pluginUnload() {
		Plugin::deleteInstance();
	}
}

/**
 * @file
 * $Id: main.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */