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
#include <luabind/object.hpp>

namespace dcpp {
class Client;
class CriticalSection;
class UserConnection;
class LuaScript;

class ScriptManager : public Singleton<ScriptManager>, private SettingsManagerListener {
public:
	typedef vector<LuaScript*> Scripts;
	ScriptManager();
	~ScriptManager();

	void load(void (*f)(void*, const tstring&), void* p);
	void close();
	void exec();

	inline void reload() {
		//close();
		//load();
	}

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

	Scripts& getScripts() {
		return scripts;
	}

	inline void getNmdcFeats(StringList& feats) {
		feats.insert(feats.end(), nmdcFeats.begin(), nmdcFeats.end());
	}

	inline void getAdcFeats(std::string& feats) {
		for(StringList::const_iterator i = adcFeats.begin(); i != adcFeats.end(); ++i)
			feats += *i + ",";
	}

private:
	void on(SettingsManagerListener::Load, SimpleXML&) throw();
	void on(SettingsManagerListener::Save, SimpleXML&) throw();

	void addSlot(const luabind::object& o, const std::string& name);
	void remSlot(const luabind::object& o, const std::string& name);
	void addNmdcFeat(const std::string& feat);
	void addAdcFeat(const std::string& feat);

	void BindScriptManager();

	friend class Singleton<ScriptManager>;

	Scripts scripts;

	typedef vector<luabind::adl::object> Objects;
	void removeObject(Objects& c, const luabind::object& o);
	int getIndex(const std::string& name);

	Objects msgIn;
	Objects msgOut;

	Objects pmIn;
	Objects pmOut;

	Objects userIn;
	Objects userOut;

	Objects hubIn;
	Objects hubOut;

	Objects connIn;
	Objects connOut;

	Objects cfgLoad;
	Objects cfgSave;

	void freeObjects() {
		msgIn.clear();
		msgOut.clear();
		pmIn.clear();
		pmOut.clear();
		userIn.clear();
		userOut.clear();
		hubIn.clear();
		hubOut.clear();
		connIn.clear();
		connOut.clear();
		cfgLoad.clear();
		cfgSave.clear();
	}

	StringList nmdcFeats;
	StringList adcFeats;

	static CriticalSection cs;
	static lua_State* parser;

};
} // namespace dcpp

#endif //RSXPLUSPLUS_SCRIPT_MANAGER
