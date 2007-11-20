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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "Singleton.h"
#include "PluginAPI.h"
#include "version.h"

class Plugin : public PluginAPI, public Singleton<Plugin> {
public:
	Plugin();
	virtual ~Plugin();

	const std::wstring getPluginName() const { return _T(PLUGIN_NAME); }
	const std::wstring getPluginVersion() const { return _T(PLUGIN_VERSION); }
	const std::wstring getPluginDescription() const { return _T(PLUGIN_DESCRIPTION); }
	const std::wstring getPluginAuthor() const { return _T(PLUGIN_AUTHOR); }
	const int getPluginID() const { return PLUGIN_ID; }
	const double getUsedAPIVersion() const { return PLUGIN_USED_API_VERSION; }
	HBITMAP getPluginIcon();

	void onLoad();
	void onUnload();
	void setDefaults();

	void onToolbarClick();
	bool onHubEnter(Client* client, const wstring& aMessage);
	bool onHubMessage(Client* client, const wstring& aMessage);

	void setInstance(HINSTANCE& i) { hInstance = i; }
private:
	HINSTANCE hInstance;
	friend class Singleton<Plugin>;
};
#endif

/**
 * @file
 * $Id: Plugin.h 42 2007-10-31 18:27:40Z adrian_007 $
 */