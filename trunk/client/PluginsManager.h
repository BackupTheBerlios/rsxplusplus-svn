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

#ifndef RSXPLUSPLUS_PLUGINS_MANAGER
#define RSXPLUSPLUS_PLUGINS_MANAGER

#include "Singleton.h"
#include "TimerManager.h"
#include "Thread.h"
#include "Socket.h"
#include "Client.h"
#include "ClientManagerListener.h"
#include "SettingsManager.h"

#include "sdk/interfaces/PluginInfo.hpp"
#include "sdk/interfaces/Core.hpp"

namespace dcpp {
class Plugin;
class Exception;

class PluginsManager : public Singleton<PluginsManager>, private TimerManagerListener, private SettingsManagerListener,
	protected interfaces::Core, protected interfaces::Memory, protected interfaces::Utils
{
public:
	PluginsManager();
	~PluginsManager();

	void init(void (*f)(void*, const tstring&), void* p);
	inline void init() {
		init(0, 0);
	}

	void load();
	void close();

	void getPluginsInfo(std::list<dcpp::interfaces::PluginInfo*>& p);

	void eventUserConnectionCreated(UserConnection* uc);
	void eventUserConnectionDestroyed(UserConnection* uc);

private:
	typedef std::list<Plugin*> Plugins;

	void loadPlugin(Plugin*& p, HMODULE dll) throw(Exception);
	void on(TimerManagerListener::Second, uint64_t /*tick*/) throw();
	void on(TimerManagerListener::Minute, uint64_t /*tick*/) throw();
	void on(SettingsManagerListener::Load, SimpleXML&) throw();
	void on(SettingsManagerListener::Save, SimpleXML&) throw();

	CriticalSection cs;
	Plugins plugins;
	//Socket udp;

	class ProxyClientManagerListener : public ClientManagerListener {
	public:
		ProxyClientManagerListener(interfaces::HubManagerListener* _i) : i(_i) { }
		interfaces::HubManagerListener* i;
	private:
		void on(ClientManagerListener::UserUpdated, const OnlineUser& ou) throw() {
			i->onHubManager_UserUpdated(const_cast<OnlineUser*>(&ou));
		}
		void on(ClientManagerListener::ClientConnected, const Client* c) throw() {
			i->onHubManager_HubConnected(const_cast<Client*>(c));
		}
		void on(ClientManagerListener::ClientUpdated, const Client* c) throw() { 
			i->onHubManager_HubUpdated(const_cast<Client*>(c));
		}
		void on(ClientManagerListener::ClientDisconnected, const Client* c) throw() { 
			i->onHubManager_HubDisconnected(const_cast<Client*>(c));
		}
	};

	typedef std::deque<ProxyClientManagerListener*> ProxyClientManagerSet;
	typedef std::deque<interfaces::ConnectionManagerListener*> CMInterfacesSet;

	ProxyClientManagerSet cmSet;
	CMInterfacesSet ucSet;

	// core
	void log(const char* msg);
	void addEventListener(interfaces::HubManagerListener* listener);
	void addEventListener(interfaces::ConnectionManagerListener* listener);

	void remEventListener(interfaces::HubManagerListener* listener);
	void remEventListener(interfaces::ConnectionManagerListener* listener);

	const char* getPluginSetting(const char* key);
	void setPluginSetting(const char* key, const char* value);

	interfaces::Memory* getMemoryManager() { return this; }
	interfaces::Utils* getUtils() { return this; }

	// memory
	char* alloc(size_t size);
	void free(char* data);
	interfaces::string* getString(const char* buf = 0);
	void putString(interfaces::string* str);
	interfaces::stringList* getStringList(size_t size = 0);
	void putStringList(interfaces::stringList* list);
	interfaces::stringMap* getStringMap();
	void putStringMap(interfaces::stringMap* map);
	interfaces::AdcCommand* getAdcCommand(uint32_t command);
	interfaces::AdcCommand* getAdcCommand(uint32_t command, const uint32_t target, char type);
	interfaces::AdcCommand* getAdcCommand(const char* str, bool nmdc);
	void putAdcCommand(interfaces::AdcCommand* command);

	// utils
	interfaces::string* formatParams(const char* format, interfaces::stringMap* params);
	interfaces::string* convertFromWideToUtf8(const wchar_t* str);
	interfaces::string* convertFromWideToAcp(const wchar_t* str);
	interfaces::string* convertFromAcpToUtf8(const char* str);
	interfaces::string* convertFromUtf8ToAcp(const char* str);
	uint32_t toSID(const char* sid);
	interfaces::string* fromSID(uint32_t sid);
	uint32_t toFourCC(const char* cc);
	interfaces::string* fromFourCC(uint32_t cc);
	const char* getPath(int type) const;

};

} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */
