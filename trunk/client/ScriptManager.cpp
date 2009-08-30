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
#include "UserCommand.h"

namespace dcpp {

lua_State* ScriptManager::parser = NULL;
CriticalSection ScriptManager::cs;

ScriptManager::ScriptManager() {
	SettingsManager::getInstance()->addListener(this);
}

ScriptManager::~ScriptManager() {
	SettingsManager::getInstance()->removeListener(this);
	TimerManager::getInstance()->removeListener(this);
}

void ScriptManager::BindScriptManager() {
	luabind::module(parser, "dcpp") [
		luabind::class_<ScriptManager>("ScriptManager")
		.def("addListener", &ScriptManager::addSlot)
		.def("removeListener", &ScriptManager::remSlot)
		.def("addAdcHubFeature", &ScriptManager::addAdcFeat)
		.def("addNmdcHubFeature", &ScriptManager::addNmdcFeat)
		.enum_("ListenerType") [
			luabind::value("HUB_LINE_IN", HUB_LINE_IN),
			luabind::value("HUB_LINE_OUT", HUB_LINE_OUT),
			luabind::value("USER_PM_IN", USER_PM_IN),
			luabind::value("USER_PM_OUT", USER_PM_OUT),
			luabind::value("USER_CONNECTED", USER_CONNECTED),
			luabind::value("USER_DISCONNECTED", USER_DISCONNECTED),
			luabind::value("HUB_CONNECTED", HUB_CONNECTED),
			luabind::value("HUB_DISCONNECTED", HUB_DISCONNECTED),
			luabind::value("CONNECTION_LINE_IN", CONNECTION_LINE_IN),
			luabind::value("CONNECTION_LINE_OUT", CONNECTION_LINE_OUT),
			luabind::value("SETTINGS_SAVE", SETTINGS_SAVE),
			luabind::value("SETTINGS_LOAD", SETTINGS_LOAD),
			luabind::value("TIMER_ON_SECOND", TIMER_ON_SECOND),
			luabind::value("TIMER_ON_MINUTE", TIMER_ON_MINUTE),
			luabind::value("UC_ON_COMMAND", UC_ON_COMMAND)
		],

		luabind::def("getScriptManager", &ScriptManager::getInstance)
	];
}

void ScriptManager::load(void (*f)(void*, const tstring&), void* p) {
	// init listeners
	listeners.resize(LISTENERS_LAST);
	for(int i = 0; i < LISTENERS_LAST; ++i)
		listeners.push_back(Objects());

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
	LuaBindings::BindSettingsManager(parser);
	LuaBindings::BindClientManager(parser);
	LuaBindings::BindFavoriteManager(parser);

	PluginsManager::getInstance()->onLuaInit(parser);

	{
		Lock l(cs);
		StringList libs = File::findFiles(Util::getPath(Util::PATH_GLOBAL_CONFIG) + "LuaScripts" PATH_SEPARATOR_STR, "*.lua");
		for(StringIter i = libs.begin(); i != libs.end(); ++i) {
			LuaScript* scr = new LuaScript;
			scr->path = Text::toT(*i);
			scr->enabled = false;
			scr->loaded = false;
			scripts.push_back(scr);
			if(f != NULL)
				(*f)(p, Util::getFileName(scr->path));
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
		int error = luaL_loadfile(parser, Text::fromT((*i)->path).c_str()) || lua_pcall(parser, 0, 0, 0);
		if(!error) {
			try {
				luabind::call_function<void>(parser, "main");
				(*i)->loaded = true;
			} catch(const luabind::error& e) { 
				luabind::object error_msg(luabind::from_stack(e.state(), -1));
				LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
			}
		} else {
			std::string msg(lua_tostring(parser, -1));
			if(msg.empty())
				msg = "(Unknown)";
			LogManager::getInstance()->message("Lua Error (syntax): " + msg);
			lua_pop(parser, 1);
		}
	}
	if(!listeners[TIMER_ON_SECOND].empty() || !listeners[TIMER_ON_MINUTE].empty())
		TimerManager::getInstance()->addListener(this);
}

void ScriptManager::runGC() {
	Lock l(cs);
	lua_gc(parser, LUA_GCCOLLECT, 0);
}

void ScriptManager::addSlot(int type, const luabind::adl::object& o) {
	if(luabind::type(o) == LUA_TFUNCTION && type < LISTENERS_LAST && type >= 0) {
		Lock l(cs);
		listeners[type].push_back(o);
	}
}

void ScriptManager::remSlot(int type, const luabind::adl::object& o) {
	if(luabind::type(o) == LUA_TFUNCTION && type < LISTENERS_LAST && type >= 0) {
		Lock lock(cs);
		Objects& l = listeners[type];		Objects::iterator i = std::find(l.begin(), l.end(), o);
		if(i != l.end())
			l.erase(i);
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
	Objects& obj = listeners[USER_PM_IN];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
	Objects& obj = listeners[USER_PM_OUT];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
	Objects& obj = listeners[HUB_LINE_IN];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
	Objects& obj = listeners[HUB_LINE_OUT];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
	Objects& obj = listeners[USER_CONNECTED];
	if(obj.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			luabind::call_function<bool>(*i, user);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::onUserDisconnected(OnlineUser* user) {
	Objects& obj = listeners[USER_DISCONNECTED];
	if(obj.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			luabind::call_function<void>(*i, user);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::onHubConnected(Client* c) {
	Objects& obj = listeners[HUB_CONNECTED];
	if(obj.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			luabind::call_function<void>(*i, c);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::onHubDisconnected(Client* c) {
	Objects& obj = listeners[HUB_DISCONNECTED];
	if(obj.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			luabind::call_function<void>(*i, c);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

bool ScriptManager::onConnectionIn(UserConnection* uc, const std::string& line) {
	Objects& obj = listeners[CONNECTION_LINE_IN];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
	Objects& obj = listeners[CONNECTION_LINE_OUT];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
				if(stricmp(file, Text::fromT((*i)->path)) == 0) {
					(*i)->enabled = xml.getBoolChildAttrib("Enabled");
					break;
				}
			}
		}
		exec();
		xml.stepOut();
	}
	if(xml.findChild("LuaSettings")) {
		xml.stepIn();
		try {
			Lock l(cs);
			Objects& obj = listeners[SETTINGS_LOAD];
			for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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
		xml.addChildAttrib("File", Text::fromT((*j)->path));
		xml.addChildAttrib("Enabled", (*j)->enabled);
	}
	xml.stepOut();

	xml.addTag("LuaSettings");
	xml.stepIn();
	try {
		Lock l(cs);
		Objects& obj = listeners[SETTINGS_SAVE];
		for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
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

void ScriptManager::on(TimerManagerListener::Second, uint64_t tick) throw() {
	Objects& obj = listeners[TIMER_ON_SECOND];
	if(obj.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			luabind::call_function<void>(*i, tick);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

void ScriptManager::on(TimerManagerListener::Minute, uint64_t tick) throw() {
	Objects& obj = listeners[TIMER_ON_MINUTE];
	if(obj.empty()) return;
	Lock l(cs);
	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			luabind::call_function<void>(*i, tick);
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
}

bool ScriptManager::onUserCmd(Client* c, UserCommand& uc) {
	Objects& obj = listeners[UC_ON_COMMAND];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;

	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, c, boost::ref(uc));
			if(r)
				ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

bool ScriptManager::onUserCmd(OnlineUser* ou, UserCommand& uc) {
	Objects& obj = listeners[UC_ON_COMMAND];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;

	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, ou, boost::ref(uc));
			if(r)
				ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

bool ScriptManager::onUserCmd(User* u, UserCommand& uc) {
	Objects& obj = listeners[UC_ON_COMMAND];
	if(obj.empty()) return false;
	Lock l(cs);
	bool ret = false;

	for(Objects::const_iterator i = obj.begin(); i != obj.end(); ++i) {
		try {
			bool r = luabind::call_function<bool>(*i, u, boost::ref(uc));
			if(r)
				ret = true;
		} catch(const luabind::error& e) { 
			luabind::object error_msg(luabind::from_stack(e.state(), -1));
			LogManager::getInstance()->message("Lua Error: " + luabind::object_cast<std::string>(error_msg));
		}
	}
	return ret;
}

} // namespace dcpp

/**
 * $Id$
 */
