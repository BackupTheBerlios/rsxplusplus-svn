/* 
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
#include "../../client/Singleton.h"
#include "../../client/CriticalSection.h"
#include "../../client/Util.h"
#include "../../client/DCPlusPlus.h"

#include "PluginInterface.h"
#include "PluginInfo.h"

class PluginsManager : public Singleton<PluginsManager> {
public:
	typedef vector<PluginInfo*> Plugins;
	Plugins& getPlugins() { Lock l(cs); return active; }

	PluginsManager();
	~PluginsManager();

	void loadPlugins();
	void unloadPlugins();

	void startPlugins();
	void reloadPlugins() { unloadPlugins(); loadPlugins(); startPlugins(); }

	void saveSettings();
	void loadSettings();

	void setSetting(int pId, const string& n, const string& v);
	const string& getSetting(int pId, const string& n);
	void setPluginInfo(int pId, const string& pn, const string& pv, int icon);

	//events
	bool onIncommingMessage(Client* client, const string& aMsg);
	bool onOutgoingMessage(Client* client, const string& aMsg);
	void onToolbarClick(int pluginId);

private:
	void loadPlugin(const string& pPath);
	bool isLoaded(int pId);
	
	string validateName(string name) {
		string::size_type i;
		while((i = name.find(' ')) != string::npos) 
			name.erase(i, 1);
		return name;
	}

	friend class Singleton<PluginsManager>;
	CriticalSection cs;

	Plugins active;
};
#endif

/**
 * @file
 * $Id: MainFrm.cpp,v 1.20 2004/07/21 13:15:15 bigmuscle Exp
 */