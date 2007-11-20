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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <vector>
using std::vector;

#include "Singleton.h"
#include "CriticalSection.h"
#include "PluginAPI.h"

class iPluginAPICallBack : public iPluginAPI {
public:
	const std::wstring getHubName(Client* c) const;
	const std::wstring getHubUrl(Client* c) const;

	void sendHubMessage(Client* client, const std::wstring& aMsg);
	void addHubLine(Client* client, const std::wstring& aMsg, int type = 0);

	void logMessage(const std::wstring& aMsg);
	int getIntSetting(const std::string& /*sName*/);
	int64_t getInt64Setting(const std::string& /*sName*/);
	std::wstring getStringSetting(const std::string& /*sName*/);
	HWND getMainWnd();
	int getSVNRevision();
	const std::wstring getClientProfileVersion() const;
	const std::wstring getMyInfoProfileVersion() const;
	bool RegExpMatch(const std::wstring& strToMatch, const std::wstring& regexp, const std::wstring& opt = _T(""));
	bool WildcardMatch(const std::wstring& strToMatch, const std::wstring& pattern, std::wstring& delim);
	void setSetting(const std::wstring& /*pName*/, const std::wstring& /*sName*/, const std::wstring& /*sValue*/);
	const std::wstring getSetting(const std::wstring& /*pName*/, const std::wstring& /*sName*/) const;
	map<std::wstring, std::wstring> getSettings(const std::wstring& /*pName*/);
	const std::wstring getDataPath() const;
};

class PluginInfo {
public:
	typedef void (*PLUGIN_UNLOAD)();

	PluginInfo(HMODULE _h, PluginAPI* _a, int _id, PLUGIN_UNLOAD _u) : h(_h), a(_a), id(_id), pluginUnloader(_u) { };
	~PluginInfo();

	HMODULE getModule() { return h; }
	PluginAPI* getPluginAPI() { return a; }
	int getId() { return id; }
	PLUGIN_UNLOAD pluginUnloader;
private:
	HMODULE h;
	PluginAPI* a;
	int id;
};

class PluginManager : public Singleton<PluginManager> {
public:
	PluginManager();
	~PluginManager();
	typedef std::vector<PluginInfo*> PluginsMap;

	void loadPluginDir();
	void loadPlugin(const string& flname);
	void unloadPlugin(const wstring& name);
	void unloadAll();
	void reloadPlugins();
	void startPlugins();

	HWND getMainHwnd() { return mainHwnd; }
	void setHwnd(HWND hWnd) { mainHwnd = hWnd; }
	
	//host -> plugin
	bool onToolbarClick(int aId);
	bool onHubEnter(Client* client, const string& aMsg);
	bool onHubMessage(Client* client, const string& aMsg);

	void setSetting(const wstring& pName, const wstring& stgName, const wstring& stgVal);
	wstring getSetting(const wstring& pName, const wstring& stgName);
	map<wstring, wstring> getPluginSettings(const wstring& pName) {
		return settings[validateName(pName)];
	}
	PluginsMap getPlugins() { return plugins; }

private:
	friend class Singleton<PluginManager>;
	HWND mainHwnd;

	bool isLoaded(const int aPluginId);
	wstring validateName(wstring name) {
		wstring::size_type i;
		while((i = name.find(' ')) != wstring::npos) 
			name.erase(i, 1);
		return name;
	}

	typedef PluginAPI* (__cdecl *PLUGIN_LOAD)();
	typedef int (*PLUGIN_ID)();
	typedef int (*PLUGIN_API_VERSION)();

	PluginsMap plugins;

	typedef std::map<wstring, wstring> SettingItem;
	typedef std::map<wstring, SettingItem> SettingsMap;
	SettingsMap settings;

	void loadSettings();
	void saveSettings();

	CriticalSection cs;
	bool dontSave;
};
#endif

/**
 * @file
 * $Id: PluginManager.h 42 2007-10-31 18:27:40Z adrian_007 $
 */