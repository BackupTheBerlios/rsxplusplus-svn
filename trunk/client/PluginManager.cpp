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
#include "DCPlusPlus.h"

#include "PluginManager.h"
#include "Util.h"

#include "pme.h"
#include "../rsx/Wildcards.h"

#include "SettingsManager.h"
#include "ResourceManager.h"
#include "ClientProfileManager.h"
#include "LogManager.h"

#include "Client.h"
#include "SimpleXML.h"
#include "version.h"

#define API_VERSION 1004

PluginManager::PluginManager() : mainHwnd(NULL), dontSave(false) {
	loadPluginDir();
}

PluginManager::~PluginManager() {
	unloadAll();
}

PluginManager::PluginInfo::~PluginInfo(){
	if(a != NULL) {
		a->onUnload();
		a->setInterface(NULL);
	}
	pluginUnloader();
	a = NULL;
	pluginUnloader = NULL;

	if(h != NULL) {
		::FreeLibrary(h);
		h = NULL;
	}
}

void PluginManager::loadPluginDir() {
	Lock l(cs);
	StringList libs = File::findFiles(Util::getDataPath() + "Plugins\\", "*.dll");
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		loadPlugin((*i));
	}
}

void PluginManager::loadPlugin(const string& flname) {
	Lock l(cs);
	HMODULE h = LoadLibrary(Text::toT(flname).c_str());

	if(h == NULL) {
		LogManager::getInstance()->message(STRING(PLUGIN_LOAD_FAIL) + " (" + Util::getFileName(flname) + ")");
		FreeLibrary(h);
		return;
	}

	PLUGIN_ID pid = (PLUGIN_ID)GetProcAddress(h, "pluginId");
	PLUGIN_API_VERSION pav = (PLUGIN_API_VERSION)::GetProcAddress(h, "pluginAPIVersion");

	if(pid != NULL && pav != NULL) {
		int pId = pid();
		int apiVer = pav();
		if(!isLoaded(pId)) {
			if(apiVer == API_VERSION) {
				PLUGIN_LOAD api = (PLUGIN_LOAD)::GetProcAddress(h, "pluginLoad");
				PLUGIN_UNLOAD ul = (PLUGIN_UNLOAD)::GetProcAddress(h, "pluginUnload");
				if(api != NULL && ul != NULL) {
					PluginAPI* a = api();
					a->setInterface(new iPluginAPICallBack());
					plugins.push_back(new PluginInfo(h, a, pId, ul));
					return;
				}
			} else {
				LogManager::getInstance()->message(STRING(PLUGIN_USING_OLD_API) + " (" + Util::getFileName(flname) + ")");
			}
		}
	} else {
		LogManager::getInstance()->message(STRING(PLUGIN_NOT_VALID) + " (" + Util::getFileName(flname) + ")");
	}
	FreeLibrary(h);
}

void PluginManager::unloadPlugin(const wstring& name) {
	Lock l(cs);
	for(PluginsMap::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if((*i)->getPluginAPI()->getPluginName().compare(name) == 0) {
			PluginInfo* p = (*i);
			plugins.erase(i);
			if(p) {
				delete p;
				p = NULL;
			}
			return;
		}
	}
}

void PluginManager::unloadAll() {
	Lock l(cs);
	saveSettings();
	settings.clear();
	for_each(plugins.begin(), plugins.end(), DeleteFunction());
	plugins.clear();
}

void PluginManager::reloadPlugins() {
	unloadAll();
	loadPluginDir();
	startPlugins();
}

void PluginManager::startPlugins() {
	Lock l(cs);
	loadSettings();
	for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		(*i)->getPluginAPI()->onLoad();
	}
}

void PluginManager::getIcons(ToolbarPlugInfo& tmpMap) {
	Lock l(cs);
	tmpMap.clear();
	for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if((*i)->getPluginAPI()->getPluginID() > 0 && (*i)->getPluginAPI()->getPluginIcon() != NULL) {
			tmpMap.insert(make_pair((*i)->getPluginAPI()->getPluginID(), (*i)->getPluginAPI()->getPluginIcon()));
		}
	}
}

const tstring PluginManager::getPluginNameById(int aId) const {
	for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if((*i)->getPluginAPI()->getPluginID() == aId) {
			return (*i)->getPluginAPI()->getPluginName().c_str();
		}
	}
	return Util::emptyStringT;
}

bool PluginManager::onToolbarClick(int aId) {
	Lock l(cs);
	for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if((*i)->getPluginAPI()->getPluginID() == aId) {
			(*i)->getPluginAPI()->onToolbarClick();
			return true;
		}
	}
	return false;
}

bool PluginManager::onHubEnter(Client* client, const string& aMsg) {
	Lock l(cs);
	bool drop = false;
	for(PluginsMap::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if(client && (*i)->getPluginAPI()->onHubEnter(*client, Text::toT(aMsg)))
			drop = true;
	}
	return drop;
}

bool PluginManager::onHubMessage(Client* client, const string& aMsg) {
	Lock l(cs);
	bool drop = false;
	for(PluginsMap::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if(client && (*i)->getPluginAPI()->onHubMessage(*client, Text::toT(aMsg)))
			drop = true;
	}
	return drop;
}

void PluginManager::setSetting(const wstring& pName, const wstring& stgName, const wstring& stgVal) {
	if(!pName.empty() && !stgName.empty()) {
		settings[validateName(pName)][stgName] = stgVal;
	}
}

wstring PluginManager::getSetting(const wstring& pName, const wstring& stgName) {
	const wstring& validName = validateName(pName);
	if(settings.find(validName) != settings.end()) {
		if(settings[validName].find(stgName) != settings[validName].end())
			return settings[validName][stgName];
	}
	return Util::emptyStringW;
}
//private:
bool PluginManager::isLoaded(const int aPluginId) {
	Lock l(cs);
	if(plugins.size() > 0 && aPluginId > 0) {
		for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
			if(aPluginId == (*i)->getPluginAPI()->getPluginID()) {
				return true;
			}
		}
	}
	return false;
}

void PluginManager::loadSettings() {
	Lock l(cs);
	dontSave = true;
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "PluginSettings.xml", File::READ, File::OPEN).read());
		if(xml.findChild("PluginSettings")) {
			xml.stepIn();
			if(xml.findChild("Plugins")) {
				xml.stepIn();
				xml.resetCurrentChild();
				for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
					(*i)->getPluginAPI()->setDefaults();
					const wstring& validName = validateName((*i)->getPluginAPI()->getPluginName());

					if(xml.findChild(Text::fromT(validName))) {
						SettingItem tempMap = (*i)->getPluginAPI()->getSettings();
						xml.stepIn();
						for(SettingItem::iterator j = tempMap.begin(); j != tempMap.end(); ++j) {
							if(xml.findChild(Text::fromT(j->first))) {
								setSetting(validName, (j->first), Text::toT(xml.getChildData()));
							}
						}
						xml.stepOut();
					}
				}
				xml.stepOut();
			}
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("PluginManager::loadSettings: %s\n", e.getError().c_str());
	}	
	dontSave = false;
}

void PluginManager::saveSettings() {
	if(dontSave)
		return;

	Lock l(cs);
	try {
		SimpleXML xml;
		xml.addTag("PluginSettings");
		xml.stepIn();
		xml.addTag("Plugins");
		xml.stepIn();
		for(PluginsMap::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
			const wstring& validName = validateName((*i)->getPluginAPI()->getPluginName());
			xml.addTag(Text::fromT(validName));
			xml.stepIn();
			SettingItem tempMap = getPluginSettings(validName);
			for(SettingItem::const_iterator j = tempMap.begin(); j != tempMap.end(); ++j) {
				xml.addTag(Text::fromT(j->first), Text::fromT(j->second));
			}
			xml.stepOut();
		}

		xml.stepOut();
		xml.stepOut();

		string fname = Util::getConfigPath() + "PluginSettings.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("PluginManager::saveSettings: %s\n", e.getError().c_str());
	}
}
//CallBack //Hub Interface
const std::wstring iPluginAPICallBack::getHubName(Client& c) const {
	Lock l(c.cs);
	return Text::toT(c.getHubName());
}

const std::wstring iPluginAPICallBack::getHubUrl(Client& c) const {
	Lock l(c.cs);
	return Text::toT(c.getHubUrl());
}

void iPluginAPICallBack::sendHubMessage(Client& client, const std::wstring& aMsg) { 
	Lock l(client.cs);
	if(&client && client.isConnected())
		client.hubMessage(Text::fromT(aMsg));
}

void iPluginAPICallBack::addHubLine(Client& client, const std::wstring& aMsg, int type) { 
	Lock l(client.cs);
	if(&client && client.isConnected())
		client.addHubLine(Text::fromT(aMsg), type);
}
//CallBack //Random Functions Interface
void iPluginAPICallBack::logMessage(const std::wstring& aMsg) { 
	LogManager::getInstance()->message(Text::fromT(aMsg));
}

int iPluginAPICallBack::getIntSetting(const std::string& sName) {
	return SettingsManager::getInstance()->getInt(sName);
}

int64_t iPluginAPICallBack::getInt64Setting(const std::string& sName) { 
	return SettingsManager::getInstance()->getInt64(sName);
}

wstring iPluginAPICallBack::getStringSetting(const std::string& sName) {
	return Text::toT(SettingsManager::getInstance()->getString(sName));
}

HWND iPluginAPICallBack::getMainWnd() {
	return PluginManager::getInstance()->getMainHwnd();
}

int iPluginAPICallBack::getSVNRevision() {
	return SVN_REVISION;
}

const wstring iPluginAPICallBack::getClientProfileVersion() const {
	return Text::toT(ClientProfileManager::getInstance()->getProfileVersion());
}

const wstring iPluginAPICallBack::getMyInfoProfileVersion() const {
	return Text::toT(ClientProfileManager::getInstance()->getMyinfoProfileVersion());
}

bool iPluginAPICallBack::RegExpMatch(const std::wstring& strToMatch, const std::wstring& regexp, const std::wstring& opt /*= _T("")*/) {
	PME regex(regexp, opt);
	if(regex.IsValid()) {
		if(regex.match(strToMatch)) {
			return true;
		}
	}
	return false;
}

bool iPluginAPICallBack::WildcardMatch(const std::wstring& strToMatch, const std::wstring& pattern, std::wstring& delim) {
	if(Wildcard::patternMatch(strToMatch, pattern, (wchar_t)delim.c_str())) {
		return true;
	}
	return false;
}

void iPluginAPICallBack::setSetting(const std::wstring& pName, const std::wstring& sName, const std::wstring& sValue) {
	PluginManager::getInstance()->setSetting(pName, sName, sValue);
}

const std::wstring iPluginAPICallBack::getSetting(const std::wstring& pName, const std::wstring& sName) const {
	return PluginManager::getInstance()->getSetting(pName, sName);
}

map<wstring, wstring> iPluginAPICallBack::getSettings(const wstring& pName) {
	return PluginManager::getInstance()->getPluginSettings(pName);
}

const std::wstring iPluginAPICallBack::getDataPath() const {
	return Text::toT(Util::getDataPath());
}

/**
 * @file
 * $Id: PluginManager.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */