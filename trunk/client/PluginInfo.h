/*
 * Copyright (C) 2007-2008 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_PLUGIN_INFO_H
#define RSXPLUSPLUS_PLUGIN_INFO_H

#include "PluginAPI/PluginInterface.h"

namespace dcpp {

class PluginInfo {
public:
	typedef iPlugin* (__cdecl * PLUGIN_LOAD)();
	typedef void (__cdecl * PLUGIN_UNLOAD)();
	typedef void (__cdecl * PLUGIN_INFORMATION)(PluginInformation&);

	PluginInfo(HINSTANCE, PLUGIN_LOAD, PLUGIN_UNLOAD, PluginInformation&);
	~PluginInfo();

	void loadPlugin();
	void unloadPlugin();
	void setSetting(const string& aName, const string& aVal);
	const string& getSetting(const string& aName);
	StringMap& getSettings() { return settings; }

	GETSET(HMODULE, handle, Handle);

	GETSET(int, id, Id);
	GETSET(tstring, name, Name);
	GETSET(tstring, version, Version);
	GETSET(tstring, description, Description);
	GETSET(tstring, author, Author);
	GETSET(int, icon, Icon);
	GETSET(int, settingsWnd, SettingsWindow);
	GETSET(iPlugin*, _interface, Interface);

private:
	friend class PluginsManager;

	CriticalSection cs;
	StringMap settings;

	PLUGIN_LOAD load;
	PLUGIN_UNLOAD unload;
};
} // namespace dcpp
#endif // RSXPLUSPLUS_PLUGIN_INFO_H

/**
 * @file
 * $Id$
 */