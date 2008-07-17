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

#ifndef PLUGINS_MANAGER
#define PLUGINS_MANAGER

#include "Singleton.h"
#include "CriticalSection.h"
#include "Util.h"

#include "PluginAPI/ClientInterface.h"
#include "PluginAPI/UserInterface.h"
#include "PluginAPI/PluginInformation.h"

#include "PluginInfo.h"

namespace dcpp {

class PluginsManager : public Singleton<PluginsManager> {
public:
	typedef vector<PluginInfo*> Plugins;
	Plugins& getPlugins() { Lock l(cs); return active; }

	PluginsManager();
	~PluginsManager();

	void loadPlugins();
	void unloadPlugins(bool withStopFuncCall);

	void startPlugins();
	void stopPlugins();

	void saveSettings();
	void loadSettings();

	void setSetting(int pId, const string& n, const string& v);
	string getSetting(int pId, const string& n);

	// events
	bool onIncommingMessage(iClient* client, const string& aMsg);
	bool onOutgoingMessage(iClient* client, const string& aMsg);

	bool onIncommingPM(iOnlineUser* from, const string& aMsg);
	bool onOutgoingPM(iOnlineUser* to, const string& aMsg);

	bool onHubConnected(iClient* hub);
	void onHubDisconnected(iClient* hub);
	void onUserConnected(iOnlineUser* user);
	void onUserDisconnected(iOnlineUser* user);

	void onToolbarClick(int pluginId);
private:
	void loadPlugin(const string& pPath);
	bool isLoaded(int pId);

	string validateName(const string& aName) const {
		string name = aName;
		string::size_type i;
		while((i = name.find(' ')) != string::npos) 
			name.erase(i, 1);
		return name;
	}

	friend class Singleton<PluginsManager>;
	CriticalSection cs;

	Plugins active;
};
}; // namespace dcpp
#endif

/**
 * @file
 * $Id$
 */