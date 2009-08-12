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

#ifndef RSXPLUSPLUS_SCRIPT_MANAGER
#define RSXPLUSPLUS_SCRIPT_MANAGER

#include "Singleton.h"
#include "TimerManager.h"

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

namespace dcpp {
	class Client;
	class CriticalSection;
	class UserConnection;
	class LuaScript;
	class UserCommand;

class ScriptManager : public Singleton<ScriptManager>, private SettingsManagerListener, private TimerManagerListener {
public:
	typedef vector<LuaScript*> Scripts;
	ScriptManager();
	~ScriptManager();

	void load(void (*f)(void*, const tstring&), void* p);
	void close();
	void runGC();

	bool onHubMsgIn(Client* c, const std::string& msg);
	bool onHubMsgOut(Client* c, const std::string& msg);
	bool onPmMsgIn(Client* hub, OnlineUser* from, OnlineUser* to, OnlineUser* replyTo, const std::string& msg, bool thirdPerson);
	bool onPmMsgOut(Client* hub, OnlineUser* to, const std::string& msg);
	void onUserConnected(OnlineUser* user);
	void onUserDisconnected(OnlineUser* user);
	void onHubConnected(Client* c);
	void onHubDisconnected(Client* c);
	bool onConnectionIn(UserConnection* uc, const std::string& line);
	bool onConnectionOut(UserConnection* uc, const std::string& line);
	bool onUserCmd(Client* c, UserCommand& uc);
	bool onUserCmd(OnlineUser* ou, UserCommand& uc);
	bool onUserCmd(User* u, UserCommand& uc);

	Scripts& getScripts() {
		return scripts;
	}

	void getNmdcFeats(StringList& feats) {
		feats.insert(feats.end(), nmdcFeats.begin(), nmdcFeats.end());
	}

	void getAdcFeats(std::string& feats) {
		for(StringList::const_iterator i = adcFeats.begin(); i != adcFeats.end(); ++i)
			feats += *i + ",";
	}

private:
	friend class Singleton<ScriptManager>;

	typedef std::vector<luabind::object> Objects;
	typedef std::vector<Objects> Listeners;

	enum {
		HUB_LINE_IN = 0,
		HUB_LINE_OUT,
		USER_PM_IN,
		USER_PM_OUT,
		USER_CONNECTED,
		USER_DISCONNECTED,
		HUB_CONNECTED,
		HUB_DISCONNECTED,
		CONNECTION_LINE_IN,
		CONNECTION_LINE_OUT,
		SETTINGS_SAVE,
		SETTINGS_LOAD,
		TIMER_ON_SECOND,
		TIMER_ON_MINUTE,
		UC_ON_COMMAND,
		LISTENERS_LAST
	};

	void BindScriptManager();
	void addSlot(int type, const luabind::object& o);
	void remSlot(int type, const luabind::object& o);
	void addNmdcFeat(const std::string& feat);
	void addAdcFeat(const std::string& feat);

	void exec();	void freeObjects() {
		//for(Listeners::const_iterator i = listeners.begin(); i != listeners.end(); ++i)
		//	(*i).clear();
		listeners.clear();
	}

	Scripts scripts;
	Listeners listeners;

	StringList nmdcFeats;
	StringList adcFeats;

	static CriticalSection cs;
	static lua_State* parser;

	void on(SettingsManagerListener::Load, SimpleXML&) throw();
	void on(SettingsManagerListener::Save, SimpleXML&) throw();

	void on(TimerManagerListener::Second, uint64_t /*tick*/) throw();
	void on(TimerManagerListener::Minute, uint64_t /*tick*/) throw();
};
} // namespace dcpp

#endif //RSXPLUSPLUS_SCRIPT_MANAGER
