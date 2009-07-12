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

#include "stdinc.h"
#include "DCPlusPlus.h"
#include "CriticalSection.h"

#include "LogManager.h"

#include "File.h"
#include "Util.h"

#include "Client.h"
#include "User.h"

#include "LuaScript.h"
#include "ScriptManager.h"
#include "PluginsManager.h"
#include "LuaBindings.h"
#include "UserConnection.h"
#include "SimpleXML.h"

/* @todo
 - option to open/close hubtab and change focus on them; option to redirect to another url - by ff
 - bind queue
*/

namespace dcpp {

lua_State* ScriptManager::parser = NULL;
CriticalSection ScriptManager::cs;

ScriptManager::ScriptManager() {
	SettingsManager::getInstance()->addListener(this);
}

ScriptManager::~ScriptManager() {
	SettingsManager::getInstance()->removeListener(this);
}

void ScriptManager::BindScriptManager() {
	luabind::module(parser, "dcpp") [
		luabind::class_<ScriptManager>("ScriptManager")
		.def("connectSlot", &ScriptManager::addSlot)
		.def("disconnectSlot", &ScriptManager::remSlot)
		.def("addAdcHubFeature", &ScriptManager::addAdcFeat)
		.def("addNmdcHubFeature", &ScriptManager::addNmdcFeat),

		luabind::class_<SettingsManager>("SettingsManager")
		.def("getString", &SettingsManager::getString)
		.def("getInt", &SettingsManager::getInt),

		luabind::class_<rsxppSettingsManager>("rsxppSettingsManager")
		.def("addAction", &rsxppSettingsManager::addAction)
		.def("getActionId", &rsxppSettingsManager::getActionId),

		luabind::def("getScriptManager", &ScriptManager::getInstance),
		luabind::def("getSettingsManager", &SettingsManager::getInstance),
		luabind::def("getRSXSettingsManager", &rsxppSettingsManager::getInstance)
	];
}

void ScriptManager::load(void (*f)(void*, const tstring&), void* p) {
	parser = lua_open();

	luaL_openlibs(parser);
	luaopen_bit(parser);

	luabind::open(parser);

	BindScriptManager();
	LuaBindings::BindDcppCore(parser);
	LuaBindings::BindFlags(parser);
	LuaBindings::BindSimpleXML(parser);
	LuaBindings::BindCID(parser);
	LuaBindings::BindUser(parser);
	LuaBindings::BindIdentity(parser);
	LuaBindings::BindOnlineUser(parser);
	LuaBindings::BindUserConnection(parser);
	LuaBindings::BindClient(parser);
	LuaBindings::BindShareManager(parser);
	LuaBindings::BindConnectionManager(parser);
	LuaBindings::BindQueueManager(parser);
	LuaBindings::BindAdcCommand(parser);
	//LuaBindings::BindTTHValue(parser);

	PluginsManager::getInstance()->onLuaInit(parser);

	{
		Lock l(cs);
		StringList libs = File::findFiles(Util::getDataPath() + "LuaScripts" PATH_SEPARATOR_STR, "*.lua");
		for(StringIter i = libs.begin(); i != libs.end(); ++i) {
			LuaScript* scr = new LuaScript;
			scr->path = *i;
			scr->enabled = false;
			scripts.push_back(scr);
			if(f != NULL)
				(*f)(p, Util::getFileName(Text::toT(*i)));
		}
	}
}

void ScriptManager::close() {
	Lock l(cs);
	for_each(scripts.begin(), scripts.end(), DeleteFunction());
	scripts.clear();

	freeObjects();
	if(parser)
		lua_close(parser);
}

void ScriptManager::exec() {
	Lock l(cs);
	for(Scripts::const_iterator i = scripts.begin(); i != scripts.end(); ++i) {
		if(!(*i)->enabled) continue;
		int error = luaL_loadfile(parser, (*i)->path.c_str()) || lua_pcall(parser, 0, 0, 0);
		if(!error) {
			try {
				luabind::call_function<void>(parser, "main");
			} catch(const luabind::error& e) { 
				luabind::object error_msg(luabind::from_stack(e.state(), -1));
				LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
			}
		}
	}
}

void ScriptManager::runGC() {
	Lock l(cs);
	lua_gc(parser, LUA_GCCOLLECT, 0);
}
void ScriptManager::removeObject(Objects& c, const luabind::object& o) {
	Objects::iterator i = std::find(c.begin(), c.end(), o);
	if(i != c.end()) c.erase(i);
}

int ScriptManager::getIndex(const std::string& name) {
	if(stricmp(name, "MessageIn") == 0)
		return 1;
	if(stricmp(name, "MessageOut") == 0)
		return 2;
	if(stricmp(name, "PrivateMessageIn") == 0)
		return 3;
	if(stricmp(name, "PrivateMessageOut") == 0)
		return 4;
	if(stricmp(name, "UserConnected") == 0)
		return 5;
	if(stricmp(name, "UserDisconnected") == 0)
		return 6;
	if(stricmp(name, "HubConnected") == 0)
		return 7;
	if(stricmp(name, "HubDisconnected") == 0)
		return 8;
	if(stricmp(name, "UserConnectionIn") == 0)
		return 9;
	if(stricmp(name, "UserConnectionOut") == 0)
		return 10;
	if(stricmp(name, "ConfigLoad") == 0)
		return 11;
	if(stricmp(name, "ConfigSave") == 0)
		return 12;
	return 0;
}

void ScriptManager::addSlot(const luabind::object& o, const std::string& name) {
	if(luabind::type(o) == LUA_TFUNCTION) {
		Lock l(cs);
		int type = getIndex(name);
		switch(type) {
			case 1: msgIn.push_back(o); break;
			case 2: msgOut.push_back(o); break;
			case 3: pmIn.push_back(o); break;
			case 4: pmOut.push_back(o); break;
			case 5: userIn.push_back(o); break;
			case 6: userOut.push_back(o); break;
			case 7: hubIn.push_back(o); break;
			case 8: hubOut.push_back(o); break;
			case 9: connIn.push_back(o); break;
			case 10: connOut.push_back(o); break;
			case 11: cfgLoad.push_back(o); break;
			case 12: cfgSave.push_back(o); break;
			default: break;
		}
	}
}

void ScriptManager::remSlot(const luabind::object& o, const std::string& name) {
	if(luabind::type(o) == LUA_TFUNCTION) {
		Lock l(cs);
		int type = getIndex(name);
		switch(type) {
			case 1: removeObject(msgIn, o); break;
			case 2: removeObject(msgOut, o); break;
			case 3: removeObject(pmIn, o); break;
			case 4: removeObject(pmOut, o); break;
			case 5: removeObject(userIn, o); break;
			case 6: removeObject(userOut, o); break;
			case 7: removeObject(hubIn, o); break;
			case 8: removeObject(hubOut, o); break;
			case 9: removeObject(connIn, o); break;
			case 10: removeObject(connOut, o); break;
			case 11: removeObject(cfgLoad, o); break;
			case 12: removeObject(cfgSave, o); break;
			default: break;
		}
	}
}

void ScriptManager::addNmdcFeat(const std::string& feat) {
	Lock l(cs);
	nmdcFeats.push_back(feat);
}

void ScriptManager::addAdcFeat(const std::string& feat) {
	Lock l(cs);
	adcFeats.push_back(feat);
}

bool ScriptManager::onPmMsgIn(Client* hub, OnlineUser* from, OnlineUser* to, OnlineUser* replyTo, const std::string& msg, bool thirdPerson) {
	if(pmIn.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = pmIn.begin(); i != pmIn.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, hub, from, to, replyTo, boost::ref(msg), thirdPerson);
			if(r) ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

bool ScriptManager::onPmMsgOut(Client* hub, OnlineUser* to, const std::string& msg) {
	if(pmOut.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = pmOut.begin(); i != pmOut.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, hub, to, boost::ref(msg));
			if(r) ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

bool ScriptManager::onHubMsgIn(Client* c, const std::string& msg) {
	if(msgIn.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = msgIn.begin(); i != msgIn.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, c, boost::ref(msg));
			if(r) ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

bool ScriptManager::onHubMsgOut(Client* c, const std::string& msg) {
	if(msgOut.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = msgOut.begin(); i != msgOut.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, c, boost::ref(msg));
			if(r) ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

void ScriptManager::onUserConnected(OnlineUser* user) {
	if(userIn.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = userIn.begin(); i != userOut.end(); ++i) {
		try {
			luabind::call_function<bool>(*i, user);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::onUserDisconnected(OnlineUser* user) {
	if(userOut.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = userOut.begin(); i != userOut.end(); ++i) {
		try {
			luabind::call_function<void>(*i, user);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::onHubConnected(Client* c) {
	if(hubIn.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = hubIn.begin(); i != hubIn.end(); ++i) {
		try {
			luabind::call_function<void>(*i, c);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::onHubDisconnected(Client* c) {
	if(hubOut.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = hubOut.begin(); i != hubOut.end(); ++i) {
		try {
			luabind::call_function<void>(*i, c);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

bool ScriptManager::onConnectionIn(UserConnection* uc, const std::string& line) {
	if(connIn.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = connIn.begin(); i != connIn.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, uc, boost::ref(line));
			if(r) ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

bool ScriptManager::onConnectionOut(UserConnection* uc, const std::string& line) {
	if(connOut.empty()) return false;													
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = connOut.begin(); i != connOut.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, uc, boost::ref(line));
			if(r) ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

void ScriptManager::on(SettingsManagerListener::Load, SimpleXML& xml) throw() {
	if(xml.findChild("LuaScripts")) {
		xml.stepIn();
		while(xml.findChild("Script")) {
			string file = xml.getChildAttrib("File");
			for(Scripts::iterator i = scripts.begin(); i != scripts.end(); ++i) {
				if(stricmp(file, (*i)->path) == 0) {
					(*i)->enabled = xml.getBoolChildAttrib("Enabled");
					break;
				}
			}
		}
		exec();
		xml.stepOut();
	} else if(xml.findChild("LuaSettings")) {
		xml.stepIn();
		try {
			for(Objects::const_iterator i = cfgLoad.begin(); i != cfgLoad.end(); ++i) {
				try {
					luabind::call_function<void>(*i, boost::ref(xml));
				} catch(const luabind::error& e) { 
					luabind::object error_msg(luabind::from_stack(e.state(), -1));
					LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
				}
			}
		} catch(const SimpleXMLException&) { }
		xml.stepOut();
	}
}

void ScriptManager::on(SettingsManagerListener::Save, SimpleXML& xml) throw() {
	xml.addTag("LuaScripts");
	xml.stepIn();
	for(Scripts::const_iterator j = scripts.begin(); j != scripts.end(); ++j) {
		xml.addTag("Script");
		xml.addChildAttrib("File", (*j)->path);
		xml.addChildAttrib("Enabled", (*j)->enabled);
	}
	xml.stepOut();

	xml.addTag("LuaSettings");
	xml.stepIn();
	try {
		for(Objects::const_iterator i = cfgSave.begin(); i != cfgSave.end(); ++i) {
			try {
				luabind::call_function<void>(*i, boost::ref(xml));
			} catch(const luabind::error& e) { 
				luabind::object error_msg(luabind::from_stack(e.state(), -1));
				LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
			}
		}
	} catch(const SimpleXMLException&) { }
	xml.stepOut();
}

} // namespace dcpp

/**
 * $Id: $
 */
