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

#if !defined(_WIN32 || _WIN64)
#error "PluginManager compatible only with win32/win64 platform"
#endif

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ResourceManager.h"

#include "PluginsManager.h"

#include "File.h"
#include "Text.h"
#include "Pointer.h"
#include "LogManager.h"
#include "SimpleXML.h"
#include "version.h"

// revision: 1.2.2.1
#define API_VERSION 1221

// @todo
// add download function from url
// extend interfaces
// add more events
// workaround with allocators a little bit more (maybe finally i'll force plugins to reload at run-time)

namespace dcpp {

PluginInfo::PluginInfo(HINSTANCE _h, PLUGIN_LOAD loader, PLUGIN_UNLOAD unloader, PluginInformation& _pi) : handle(_h), 
	load(loader), unload(unloader), _interface(NULL), icon(0)
{
	setName(_pi.pName);
	setVersion(_pi.pVersion);
	setDescription(_pi.pDesc);
	setAuthor(_pi.pAuthor);
	setId(_pi.pId);
	setIcon(_pi.pIconResourceId);
	setSettingsWindow(_pi.pSettingsDlgResourceId);
}

PluginInfo::~PluginInfo() {
	unloadPlugin();
	// release handlers here...
	//::FreeLibrary(handle);
}

void PluginInfo::loadPlugin() {
	if(load != NULL) {
		setInterface(load());
		load = NULL;
	}
}

void PluginInfo::unloadPlugin() {
	if(unload != NULL) {
		unload();
		unload = NULL;
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
	unloadPlugins(true);
}

void PluginsManager::loadPlugins() {
	Lock l(cs);
	StringList libs = File::findFiles(Util::getDataPath() + "Plugins" PATH_SEPARATOR_STR, "*.dll");
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		loadPlugin((*i));
	}
	libs.clear();
}

void PluginsManager::unloadPlugins(bool withStopFuncCall) {
	Lock l(cs);
	if(withStopFuncCall) {
		for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
			(*i)->unloadPlugin();
		}
	}
	for_each(active.begin(), active.end(), DeleteFunction());
	active.clear();
}

void PluginsManager::loadPlugin(const string& pPath) {
	HMODULE h = LoadLibrary(Text::toT((pPath)).c_str());
	if(h == NULL) {
		LogManager::getInstance()->message(boost::str(boost::format("Failed to load plugin! (%1%)") % Util::getFileName(pPath)));
		FreeLibrary(h);
		return;
	}

	PluginInfo::PLUGIN_LOAD loadFunc = (PluginInfo::PLUGIN_LOAD)::GetProcAddress(h, "pluginLoad");
	PluginInfo::PLUGIN_UNLOAD unloadFunc = (PluginInfo::PLUGIN_UNLOAD)::GetProcAddress(h, "pluginUnload");
	PluginInfo::PLUGIN_INFORMATION plugInfo = (PluginInfo::PLUGIN_INFORMATION)::GetProcAddress(h, "pluginInfo");

	if(plugInfo != NULL && loadFunc != NULL && unloadFunc != NULL) {
		PluginInformation pi;
		memzero(&pi, sizeof(pi));

		plugInfo(pi);
		if(pi.pApiVersion == API_VERSION) {
			if(!isLoaded(pi.pId)) {
				//ok, we've got a valid (not started yet) plugin... ;)
				active.push_back(new PluginInfo(h, loadFunc, unloadFunc, pi));
				return;
			} else { LogManager::getInstance()->message(boost::str(boost::format("Plugin already loaded! (%1%)") % Util::getFileName(pPath))); }
		} else { LogManager::getInstance()->message(boost::str(boost::format("Plugin is using old API revision! (%1%)") % Util::getFileName(pPath))); }
	} else { LogManager::getInstance()->message(boost::str(boost::format("%1% is not a valid %2% plugin!") % Util::getFileName(pPath) % APPNAME)); }
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

void PluginsManager::setSetting(int pId, const string &n, const string &v) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pId) {
			(*i)->setSetting(n, v);
			break;
		}
	}
}

string PluginsManager::getSetting(int pId, const string& n) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pId) {
			return (*i)->getSetting(n);
		}
	}
	// just in case
	return Util::emptyString;
}
/** Events **/
bool PluginsManager::onIncommingMessage(iClient* client, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onIncomingMessage(client, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onOutgoingMessage(iClient* client, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onOutgoingMessage(client, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onIncommingPM(iOnlineUser* from, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onIncomingPM(from, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onOutgoingPM(iOnlineUser* to, const string& aMsg) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onOutgoingPM(to, aMsg.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onHubConnected(iClient* hub) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onHubConnected(hub))
			ret = true;
	}
	return ret;
}

void PluginsManager::onHubDisconnected(iClient* hub) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		(*i)->getInterface()->onHubDisconnected(hub);
	}
}

void PluginsManager::onUserConnected(iOnlineUser* user) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		(*i)->getInterface()->onUserConnected(user);
	}
}

void PluginsManager::onUserDisconnected(iOnlineUser* user) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		(*i)->getInterface()->onUserDisconnected(user);
	}
}

bool PluginsManager::onUserConnectionIn(iUserConnection* conn, const string& aLine) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onUserConnectionIn(conn, aLine.c_str()))
			ret = true;
	}
	return ret;
}

bool PluginsManager::onUserConnectionOut(iUserConnection* conn, const string& aLine) {
	Lock l(cs);
	bool ret = false;
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getInterface()->onUserConnectionOut(conn, aLine.c_str()))
			ret = true;
	}
	return ret;
}

void PluginsManager::onToolbarClick(int pluginId, HWND btnHwnd) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if((*i)->getId() == pluginId) {
			(*i)->getInterface()->onMainWndEvent(0, btnHwnd);
			break;
		}
	}
}

void PluginsManager::onSettingsDlgClose(int pId, HWND pagehWnd) {
	Lock l(cs);
	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		if(pId == (*i)->getId()) {
			(*i)->getInterface()->onMainWndEvent(2, pagehWnd);
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
			const string& validName = validateName(Text::fromT((*i)->getName()));
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
					const string& validName = validateName(Text::fromT((*i)->getName()));
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

	for(Plugins::const_iterator i = active.begin(); i != active.end(); ++i) {
		(*i)->getInterface()->onSettingsEvent(0);
	}
}
}; // namespace dcpp

/**
 * @file
 * $Id$
 */