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

#include "../../windows/stdafx.h"
#include "../../client/DCPlusPlus.h"

#include "PluginsManager.h"
#include "PluginAPI.h"

#include "../../client/File.h"
#include "../../client/Text.h"
#include "../../client/Pointer.h"
#include "../../client/LogManager.h"
#include "../../client/SimpleXML.h"

// revision: 1.1.0.0
#define API_VERSION 1100

// @todo
// add download function from url
// fix some breakpoints caused by weird allocation

PluginInfo::PluginInfo(HINSTANCE _h, PLUGIN_LOAD loader, PLUGIN_UNLOAD unloader, int _id) : id(_id), handle(_h), 
	load(loader), unload(unloader), icon(NULL), name(Util::emptyString), version(Util::emptyString), plugin(NULL)
{
	//nothing iteresting at this time... wait for all
}

PluginInfo::~PluginInfo() {
	unloadPlugin();
	/** I should now release handler, but then I'll get access violation
	 *  so leave them, on exit application will release all this stuff
	 *  (crt have sth do deal with it...)
	 */
}

void PluginInfo::loadPlugin() {
	if(load != NULL) {
		setPlugin(load());
		load = NULL;
	}
}

void PluginInfo::unloadPlugin() {
	if(unload != NULL && plugin != NULL) {
		unload();
		unload = NULL;
		plugin = NULL;
	}
}

void PluginInfo::setSetting(const string& aName, const string& aVal) {
	Lock l(cs);
	StringMap::iterator i = settings.find(aName);
	if(i != settings.end()) {
		i->second = aVal;
		return;
	} else {
		settings.insert(make_pair(aName, aVal));
	}
}

const string& PluginInfo::getSetting(const string& aName) {
	Lock l(cs);
	StringMap::const_iterator i = settings.find(aName);
	if(i != settings.end())
		return i->second;
	return Util::emptyString;
}

PluginsManager::PluginsManager() {
	loadPlugins();
}

PluginsManager::~PluginsManager() {
	//make some cleanup
	Lock l(cs);
	stopPlugins();
	for_each(active.begin(), active.end(), DeleteFunction());
	active.clear();
}

void PluginsManager::loadPlugins() {
	Lock l(cs);
	StringList libs = File::findFiles(Util::getDataPath() + "Plugins" PATH_SEPARATOR_STR, "*.dll");
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		loadPlugin((*i));
	}
	libs.clear();
}

void PluginsManager::loadPlugin(const string& pPath) {
	HMODULE h = LoadLibrary(Text::toT((pPath)).c_str());
	if(h == NULL) {
		LogManager::getInstance()->message(STRING(PLUGIN_LOAD_FAIL) + "(" + Util::getFileName(pPath) + ")");
		FreeLibrary(h);
		return;
	}

	//first of all check used interface api version
	typedef int (__cdecl * PLUGIN_API_VER)();
	PLUGIN_API_VER apiV = (PLUGIN_API_VER)::GetProcAddress(h, "pluginAPI");

	//make 3 calls, 1th for id, 2th for load func, 3th for unload func
	PluginInfo::PLUGIN_ID pid = (PluginInfo::PLUGIN_ID)::GetProcAddress(h, "pluginId");
	PluginInfo::PLUGIN_LOAD loadFunc = (PluginInfo::PLUGIN_LOAD)::GetProcAddress(h, "pluginLoad");
	PluginInfo::PLUGIN_UNLOAD unloadFunc = (PluginInfo::PLUGIN_UNLOAD)::GetProcAddress(h, "pluginUnload");

	if(apiV != NULL && pid != NULL && loadFunc != NULL && unloadFunc != NULL) {
		int apiVersion = apiV();
		if(apiVersion == API_VERSION) {
			int pID = pid();
			if(!isLoaded(pID)) {
				//ok, we've got a valid - not yet started - plugin... ;)
				active.push_back(new PluginInfo(h, loadFunc, unloadFunc, pID));
				return;
			} else { LogManager::getInstance()->message(STRING(PLUGIN_ALREADY_LOADED) + "(" + Util::getFileName(pPath) + ")"); }
		} else { LogManager::getInstance()->message("Plugin is using old version of API! (" + Util::getFileName(pPath) + ")"); }
	} else { LogManager::getInstance()->message(STRING(PLUGIN_NOT_VALID) + "(" + Util::getFileName(pPath) + ")"); }
	FreeLibrary(h);
	h = NULL;
}

bool PluginsManager::isLoaded(int pId) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pId)
			return true;
	}
	return false;
}

void PluginsManager::startPlugins() {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		(*i)->loadPlugin();
	}
	loadSettings();
}

void PluginsManager::stopPlugins() {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		(*i)->unloadPlugin();
	}
	saveSettings();
}

void PluginsManager::setPluginInfo(int pId, const string& pn, const string& pv, int icon) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pId) {
			PluginInfo* info = (*i);
			info->setName(pn);
			info->setVersion(pv);
			if(icon > 0) {
				info->setIcon(icon);
			}
			break;
		}
	}
}

void PluginsManager::setSetting(int pId, const string &n, const string &v) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pId) {
			(*i)->setSetting(n, v);
			break;
		}
	}
}

const string& PluginsManager::getSetting(int pId, const string& n) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pId) {
			return (*i)->getSetting(n);
		}
	}
	//just in case
	return Util::emptyString;
}

bool PluginsManager::onIncommingMessage(iClient* client, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getPlugin() == NULL) continue;
		if((*i)->getPlugin()->onIncommingMessage(client, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onOutgoingMessage(iClient* client, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getPlugin() == NULL) continue;
		if((*i)->getPlugin()->onOutgoingMessage(client, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onIncommingPM(iUser* from, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getPlugin() == NULL) continue;
		if((*i)->getPlugin()->onIncommingPM(from, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onOutgoingPM(iUser* to, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getPlugin() == NULL) continue;
		if((*i)->getPlugin()->onOutgoingPM(to, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

void PluginsManager::onToolbarClick(int pluginId) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getPlugin() == NULL) continue;
		if((*i)->getId() == pluginId) {
			(*i)->getPlugin()->onToolbarClick();
			break;
		}
	}
}

void PluginsManager::saveSettings() {
	Lock l(cs);
	//don't append settings when there is no plugin
	if(!active.size())
		return;
	try {
		SimpleXML xml;
		xml.addTag("PluginSettings");
		xml.stepIn();
		xml.addTag("Plugins");
		xml.stepIn();
		for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
			const string& validName = validateName((*i)->getName());
			xml.addTag(validName);
			xml.stepIn();
			StringMap& smap = (*i)->getSettings();
			for(StringMap::const_iterator j = smap.begin(); j != smap.end(); ++j) {
				xml.addTag(j->first, j->second);
			}
			smap.clear();
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

void PluginsManager::loadSettings() {
	Lock l(cs);
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "PluginSettings.xml", File::READ, File::OPEN).read());
		if(xml.findChild("PluginSettings")) {
			xml.stepIn();
			if(xml.findChild("Plugins")) {
				xml.stepIn();
				xml.resetCurrentChild();
				for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
					const string& validName = validateName((*i)->getName());
					if(xml.findChild(validName)) {
						StringMap& smap = (*i)->getSettings();
						xml.stepIn();
						for(StringMap::iterator j = smap.begin(); j != smap.end(); ++j) {
							if(xml.findChild(j->first)) {
								j->second = xml.getChildData();
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

	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i)
		(*i)->getPlugin()->onSettingsLoaded();
}

/**
 * @file
 * $Id$
 */