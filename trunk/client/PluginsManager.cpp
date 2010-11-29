/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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

#include "PluginsManager.h"
#include "Plugin.h"

#include "Client.h"
#include "User.h"
#include "UserConnection.h"
#include "AdcCommand.h"

#include "LogManager.h"
#include "version.h"
#include "sdk/StringImpl.hpp"
#include "sdk/AdcCommandImpl.hpp"

namespace dcpp {

PluginsManager::PluginsManager() {
//	udp.create(Socket::TYPE_UDP);

	TimerManager::getInstance()->addListener(this);
	SettingsManager::getInstance()->addListener(this);
}

PluginsManager::~PluginsManager() {
//	udp.shutdown();

	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
	
	close();
	std::for_each(cmSet.begin(), cmSet.end(), DeleteFunction());
	cmSet.clear();
}

void PluginsManager::loadPlugin(Plugin*& p, HINSTANCE dll) throw(Exception) {
	typedef interfaces::PluginInfo* (__stdcall *plugInfo)(unsigned long long, int);
	p = new Plugin(dll);

	Plugin::PluginLoad pLoad = reinterpret_cast<Plugin::PluginLoad>(GetProcAddress(dll, "pluginLoad"));
	Plugin::PluginUnload pUnload = reinterpret_cast<Plugin::PluginUnload>(GetProcAddress(dll, "pluginUnload"));
	plugInfo pInfo = reinterpret_cast<plugInfo>(GetProcAddress(dll, "pluginInfo"));

	if(!pLoad)
		throw Exception("Missing pluginLoad function");
	if(!pUnload)
		throw Exception("Missing pluginUnload function");
	if(!pInfo)
		throw Exception("Missing pluginInfo function");

	p->pluginLoad = pLoad;
	p->pluginUnload = pUnload;

	interfaces::PluginInfo* nfo = pInfo(SDK_VERSION, SVN_REVISION);
	if(!nfo)
		throw Exception("Missing plugin information");

	p->info = nfo;

	if(p->info->guid == 0 || strlen(p->info->guid) < 32) {
		throw Exception("GUID not set or not valid");
	}

	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if(strcmp(p->info->guid, (*i)->info->guid) == 0)
			throw Exception("Plugin with same GUID is already loaded");
	}

	if(VER_MAJ(nfo->sdkVersion) != VER_MAJ(SDK_VERSION) || 
		VER_MIN(nfo->sdkVersion) != VER_MIN(SDK_VERSION) ||
		VER_REV(nfo->sdkVersion) != VER_REV(SDK_VERSION))
		throw Exception("Plugin is compiled with old version of PluginSDK");

	if(pLoad(this, (Plugin*)p) != 0) {
		pUnload();
		throw Exception("Unknown exception while calling pluginLoad function");
	}
}

void PluginsManager::init(void (*f)(void*, const tstring&), void* pv) {
	Lock l(cs);
	StringList libs = File::findFiles(Util::getPath(Util::PATH_GLOBAL_CONFIG) + "Plugins" PATH_SEPARATOR_STR, "*.dll");
	for(StringIter i = libs.begin(); i != libs.end(); ++i) {
		const tstring& fname = Text::toT(*i);
		HINSTANCE dll = LoadLibrary(fname.c_str());
		if(dll) {
			Plugin* p = 0;
			try {
				loadPlugin(p, dll);
				plugins.push_back(p);
				if(f && pv)
					(*f)(pv, Text::toT(Util::getFileName(*i)));
			} catch(const Exception& e) {
				// do a cleanup
				delete p;
				p = 0;
				FreeLibrary(dll);
				LogManager::getInstance()->message(Util::getFileName(*i) + ": " + e.getError());
			}
		}
	}
}

void PluginsManager::getPluginsInfo(std::list<dcpp::interfaces::PluginInfo*>& p) {
	Lock l(cs);
	for(Plugins::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
		p.push_back((*i)->info);
}

void PluginsManager::load() {

}

void PluginsManager::close() {
	Lock l(cs);
	for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		Plugin* p = *i;
		if(p->pluginUnload)
			p->pluginUnload();
		FreeLibrary((HMODULE)p->handle);
		
		i = plugins.erase(i);
		delete p;
		p = 0;
	}
}

void PluginsManager::log(const char* msg) {
	LogManager::getInstance()->message(string(msg));
}

void PluginsManager::addEventListener(interfaces::HubManagerListener* listener) {
	Lock l(cs);
	for(ProxyClientManagerSet::iterator i = cmSet.begin(); i != cmSet.end(); ++i) {
		if((*i)->i == listener)
			return;
	}

	ProxyClientManagerListener* px = new ProxyClientManagerListener(listener);
	ClientManager::getInstance()->addListener(px);
	cmSet.push_back(px);
}

void PluginsManager::remEventListener(interfaces::HubManagerListener* listener) {
	Lock l(cs);
	for(ProxyClientManagerSet::iterator i = cmSet.begin(); i != cmSet.end(); ++i) {
		ProxyClientManagerListener* px = *i;
		if(px->i == listener) {
			ClientManager::getInstance()->removeListener(px);
			delete px;
			cmSet.erase(i);
			break;
		}
	}
}

void PluginsManager::addEventListener(interfaces::ConnectionManagerListener* listener) {
	Lock l(cs);
	for(CMInterfacesSet::iterator i = ucSet.begin(); i != ucSet.end(); ++i) {
		if((*i) == listener)
			return;
	}
	ucSet.push_back(listener);
}

void PluginsManager::remEventListener(interfaces::ConnectionManagerListener* listener) {
	Lock l(cs);
	CMInterfacesSet::iterator i = std::find(ucSet.begin(), ucSet.end(), listener);
	if(i != ucSet.end())
		ucSet.erase(i);
}

const char* PluginsManager::getPluginSetting(const char* key) {
	return rsxppSettingsManager::getInstance()->getExtSetting(key).c_str();
}

void PluginsManager::setPluginSetting(const char* key, const char* value) {
	rsxppSettingsManager::getInstance()->setExtSetting(key, value);
}

void PluginsManager::eventUserConnectionCreated(UserConnection* uc) {
	Lock l(cs);
	for(CMInterfacesSet::iterator i = ucSet.begin(); i != ucSet.end(); ++i) {
		(*i)->onConnectionManager_ConnectionCreated(static_cast<interfaces::UserConnection*>(uc));
	}
}

void PluginsManager::eventUserConnectionDestroyed(UserConnection* uc) {
	Lock l(cs);
	for(CMInterfacesSet::iterator i = ucSet.begin(); i != ucSet.end(); ++i) {
		(*i)->onConnectionManager_ConnectionDestroyed(static_cast<interfaces::UserConnection*>(uc));
	}
}

char* PluginsManager::alloc(size_t size) {
	if(size > 0) {
		char* c = new char[size];
		memzero(c, size);
		return c;
	}
	return 0;
}

void PluginsManager::free(char* data) {
	if(data) {
		delete data;
	}
}

interfaces::string* PluginsManager::getString(const char* buf /*= 0*/) {
	if(buf)
		return new StringImpl();
	return new StringImpl(buf);
}

void PluginsManager::putString(interfaces::string* str) {
	StringImpl* s = dynamic_cast<StringImpl*>(str);
	if(s)
		delete s;
}

interfaces::stringList* PluginsManager::getStringList(size_t size /*= 0*/) {
	return new StringListImpl(size);
}

void PluginsManager::putStringList(interfaces::stringList* list) {
	StringListImpl* l = dynamic_cast<StringListImpl*>(list);
	if(l)
		delete l;
}

interfaces::stringMap* PluginsManager::getStringMap() {
	return new StringMapImpl();
}

void PluginsManager::putStringMap(interfaces::stringMap* map) {
	StringMapImpl* sm = dynamic_cast<StringMapImpl*>(map);
	if(sm)
		delete sm;
}

interfaces::AdcCommand* PluginsManager::getAdcCommand(uint32_t command) {
	return new AdcCommandImpl(command);
}

interfaces::AdcCommand* PluginsManager::getAdcCommand(uint32_t command, const uint32_t target, char type) {
	return new AdcCommandImpl(command, target, type);
}

interfaces::AdcCommand* PluginsManager::getAdcCommand(const char* str, bool nmdc) {
	return new AdcCommandImpl(str, nmdc);
}

void PluginsManager::putAdcCommand(interfaces::AdcCommand* command) {
	AdcCommandImpl* c = dynamic_cast<AdcCommandImpl*>(command);
	if(c)
		delete c;
}

interfaces::string* PluginsManager::formatParams(const char* format, interfaces::stringMap* params) {
	StringMapImpl* sm = dynamic_cast<StringMapImpl*>(params);
	if(sm) {
		string formatted = Util::formatParams(format, sm->getMap(), false);
		return new StringImpl(formatted);
	}
	return 0;
}

interfaces::string* PluginsManager::convertFromWideToUtf8(const wchar_t* str) {
	StringImpl* s = new StringImpl();
	Text::wideToUtf8(str, s->getString());
	return s;
}

interfaces::string* PluginsManager::convertFromWideToAcp(const wchar_t* str) {
	StringImpl* s = new StringImpl();
	Text::wideToAcp(str, s->getString());
	return s;
}

interfaces::string* PluginsManager::convertFromAcpToUtf8(const char* str) {
	StringImpl* s = new StringImpl();
	Text::acpToUtf8(str, s->getString());
	return s;
}

interfaces::string* PluginsManager::convertFromUtf8ToAcp(const char* str) {
	StringImpl* s = new StringImpl();
	Text::utf8ToAcp(str, s->getString());
	return s;
}

uint32_t PluginsManager::toSID(const char* sid) {
	return AdcCommand::toSID(sid);
}

interfaces::string* PluginsManager::fromSID(uint32_t sid) {
	return new StringImpl(AdcCommand::fromSID(sid));
}

uint32_t PluginsManager::toFourCC(const char* cc) {
	return AdcCommand::toFourCC(cc);
}

interfaces::string* PluginsManager::fromFourCC(uint32_t cc) {
	return new StringImpl(AdcCommand::fromFourCC(cc));
}

void PluginsManager::on(TimerManagerListener::Second, uint64_t tick) throw() {

}

void PluginsManager::on(TimerManagerListener::Minute, uint64_t tick) throw() {

}

void PluginsManager::on(SettingsManagerListener::Load, SimpleXML& xml) throw() {

}

void PluginsManager::on(SettingsManagerListener::Save, SimpleXML& xml) throw() {

}

} // namespace dcpp

/**
 * @file
 * $Id$
 */
