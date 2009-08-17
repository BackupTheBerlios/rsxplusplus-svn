/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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
#include "CriticalSection.h"

#include "sdk/dcpp.h"
#include "sdk/events.h"

struct lua_State;

namespace dcpp {
class Plugin;
class Client;
class OnlineUser;
class UserConnection;

class PluginsManager : public Singleton<PluginsManager> {
public:
	PluginsManager();
	~PluginsManager();

	void init(void (*f)(void*, const tstring&), void* p);
	void load();

	void initClose();

	void getPluginsInfo(std::list<dcppPluginInformation*>& p);

	bool onHubMsgIn(Client* hub, const char* msg);
	bool onHubMsgOut(Client* hub, const char* msg);
	bool onPMIn(Client* hub, OnlineUser* from, OnlineUser* to, OnlineUser* replyTo, const char* msg, bool thirdPerson);
	bool onPMOut(Client* hub, OnlineUser* to, const char* msg);
	void onHubConnecting(Client* hub);
	void onHubConnected(Client* hub);
	void onHubDisconnected(Client* hub);
	void onHubRedirect(Client* hub, const char* newUrl);
	void onUserConnected(OnlineUser* ou);
	void onUserDisconnected(OnlineUser* ou);
	void onLuaInit(lua_State*);
	void onConfigChange();
	bool onUserConnectionLineIn(UserConnection* uc, const char* line);
	bool onUserConnectionLineOut(UserConnection* uc, const char* line);

private:
	typedef vector<Plugin*> Plugins;
	struct PlugListener {
		PlugListener(dcppListenerFunc _f, dcpp_ptr_t d = 0) : f(_f), pData(d) { };

		dcppListenerFunc f;
		dcpp_ptr_t pData;
		template<typename T1, typename T2>
		int call(T1 p1, T2 p2) {
			return f((dcpp_ptr_t)p1, (dcpp_ptr_t)p2, pData);
		}
	};
	typedef std::list<PlugListener*> Listener;

	static dcpp_ptr_t __stdcall callFunc(int type, dcpp_ptr_t p1, dcpp_ptr_t p2, dcpp_ptr_t p3);
	static void* __stdcall addListener(int type, dcppListenerFunc f, dcpp_ptr_t pd);
	static void  __stdcall removeListener(void* ptr);

	template<bool breakAtFirst, typename T1, typename T2>
	int call(Listener& l, T1 p1, T2 p2);

	void* addPlugListener(int type, dcppListenerFunc f, dcpp_ptr_t pd);
	void remPlugListener(PlugListener* l);
	void deleteListener(Listener& l, PlugListener* ls);

	dcppFunctions* dcpp_func;

	CriticalSection cs;

	Plugins plugins;

	Listener hubEvents;
	Listener userEvents;
	Listener coreEvents;
	Listener connEvents;
};
} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */
