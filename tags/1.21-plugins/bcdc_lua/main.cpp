/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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


#include "stdinc.hpp"
#include "LuaManager.hpp"

static void callalert (lua_State *L, int status) {
	if (status != 0) {
		lua_getglobal(L, "_ALERT");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		}
		else {  /* no _ALERT function; print it on stderr */
			const char* msg = lua_tostring(L, -2);
			size_t len = strlen(msg);
			char* message = new char[len+12];
			memset(message, 0, len+12);
			strcat(message, "LUA ERROR: ");
			strncat(message, msg, len);

			core->log(message);

			delete[] message;
			lua_pop(L, 2);  /* remove error message and _ALERT */
		}
	}
}

static int aux_do (lua_State *L, int status) {
  if (status == 0) {  /* parse OK? */
    status = lua_pcall(L, 0, LUA_MULTRET, 0);  /* call main */
  }
  callalert(L, status);
  return status;
}

static lua_State* gL;

LUALIB_API int lua_dofile (lua_State *L, const char *filename) {
  return aux_do(L, luaL_loadfile(L, filename));
}

LUALIB_API int lua_dobuffer (lua_State *L, const char *buff, size_t size,
                          const char *name) {
  return aux_do(L, luaL_loadbuffer(L, buff, size, name));
}

LUALIB_API int lua_dostring (lua_State *L, const char *str) {
  return lua_dobuffer(L, str, strlen(str), str);
}

extern int lua_dostring(const char* str) {
	return lua_dostring(gL, str);
}

static bool MakeCallRaw(const char* table, const char* method, int args = 0, int ret = 0) throw() {
	lua_getglobal(gL, table);				// args + 1
	lua_pushstring(gL, method);				// args + 2
	if (lua_istable(gL, -2)) {
		lua_gettable(gL, -2);				// args + 2
		lua_remove(gL, -2);					// args + 1
		lua_insert(gL, 1);					// args + 1
		if(lua_pcall(gL, args, ret, 0) == 0) {
			return true;
		}
		const char* msg = lua_tostring(gL, -1);
		const char* cuteMsg = strstr(msg, "\\scripts\\");
		if(cuteMsg)
			msg = cuteMsg+9;
		const int msgSize = 1024*5;
		char message[msgSize];
		memset(message, 0, msgSize);
		strcat(message, "Lua Error: ");
		if(msg) {
			strncat(message, msg, msgSize - 12);
		} else {
			strcat(message, " (unknown)");
		}

		core->log(message);
		lua_pop(gL, 1);
	} else {
		lua_settop(gL, 0);
	}
	return false;
}

static bool GetLuaBool() {
	bool ret = false;
	if (lua_gettop(gL) > 0) {
		ret = !lua_isnil(gL, -1);
		lua_pop(gL, 1);
	}
	return ret;
}

inline void LuaPush(int i) { 
	lua_pushnumber(gL, i);
}

inline void LuaPush(const char* s) { 
	lua_pushstring(gL, s); 
}

template <typename T>
inline void LuaPush(T* p) { 
	lua_pushlightuserdata(gL, (void*)p);
}

template <typename T>
inline void LuaPush(T p) { 
	lua_pushlightuserdata(gL, (void*)p);
}

inline bool MakeCall(const char* table, const char* method, int ret = 0) {
	return MakeCallRaw(table, method, 0, ret);
}

template <typename T>
inline bool MakeCall(const char* table, const char* method, int ret, const T& t) throw() {
	//Lock l(cs);
	LuaPush(t);
	return MakeCallRaw(table, method, 1, ret);
}

template <typename T, typename T2>
inline bool MakeCall(const char* table, const char* method, int ret, const T& t, const T2& t2) throw() {
	//Lock l(cs);
	LuaPush(t);
	LuaPush(t2);
	return MakeCallRaw(table, method, 2, ret);
}

template <typename T, typename T2, typename T3>
inline bool MakeCall(const char* table, const char* method, int ret, const T& t, const T2& t2, const T3& t3) throw() {
	//Lock l(cs);
	LuaPush(t);
	LuaPush(t2);
	LuaPush(t3);
	return MakeCallRaw(table, method, 2, ret);
}

dcpp::interfaces::PluginInfo info = {
	"BCDC++ LuaScripts",
	"{338A05CC-B0A5-4533-9CF5-A7FD824AE6D2}",
	"adrian_007",
	"BCDC++ compatible " LUA_RELEASE " interface",
	"http://rsxplusplus.sf.net",
	0,
	MAKE_VER(1, 1, 0, 0),
	SDK_VERSION
};

class Plugin : 
	private dcpp::interfaces::HubListener, 
	private dcpp::interfaces::HubManagerListener,
	private dcpp::interfaces::UserConnectionListener,
	private dcpp::interfaces::ConnectionManagerListener,
	private dcpp::interfaces::TimerListener,
	private dcpp::interfaces::CoreListener
{
public:
	Plugin() {
		core->addEventListener((dcpp::interfaces::CoreListener*)this);
		core->addEventListener((dcpp::interfaces::HubManagerListener*)this);
		core->addEventListener((dcpp::interfaces::ConnectionManagerListener*)this);
	}

	~Plugin() {
		core->remEventListener((dcpp::interfaces::HubManagerListener*)this);
		core->remEventListener((dcpp::interfaces::ConnectionManagerListener*)this);
	}

private:
	void onCore_LoadComplete() throw() { 
		gL = luaL_newstate();
		luaL_openlibs(gL);

		Luna<LuaManager>::Register(gL);

		lua_pop(gL, lua_gettop(gL));

		strcpy(LuaManager::appPath, core->getUtils()->getPath(dcpp::interfaces::Paths::PATH_RESOURCES));
		strcpy(LuaManager::tempBuffer, core->getUtils()->getPath(dcpp::interfaces::Paths::PATH_RESOURCES));
		strcat(LuaManager::tempBuffer, "scripts\\startup.lua");
		strcpy(LuaManager::configPath, core->getUtils()->getPath(dcpp::interfaces::Paths::PATH_USER_CONFIG));

		lua_dofile(gL, LuaManager::tempBuffer);

		memset(LuaManager::tempBuffer, 0, TEMPBUF_SIZE);
		core->addEventListener((dcpp::interfaces::HubManagerListener*)this);
		core->addEventListener((dcpp::interfaces::ConnectionManagerListener*)this);
	}

	/*
	void onCore_UnloadPrepare() throw() {
	}
	void onHub_Connecting(dcpp::interfaces::Hub* h) throw() {
	}
	void onHub_Connected(dcpp::interfaces::Hub*) throw() {
	}
	void onHub_UserUpdated(dcpp::interfaces::Hub*, dcpp::interfaces::OnlineUser*) throw() { 
	}
	void onHub_UserRemoved(dcpp::interfaces::Hub*, dcpp::interfaces::OnlineUser*) throw() {
	}
	void onHub_Redirect(dcpp::interfaces::Hub*, const char*) throw() { 
	}
	void onHub_Failed(dcpp::interfaces::Hub*, const char*) throw() 	{ 
	}
	void onHub_HubUpdated(dcpp::interfaces::Hub*) throw() { 
	}
	void onHub_HubFull(dcpp::interfaces::Hub*) throw() 	{ 
	}
	void onHub_AccountNickTaken(dcpp::interfaces::Hub*) throw() { 
	}
	*/

	void onHub_IncomingCommand(dcpp::interfaces::Hub* h, const char* line, bool& handled) throw() { 
		if(!handled) {
			bool isAdc = strncmp(h->getHubURL(), "adc://", 6) == 0 || strncmp(h->getHubURL(), "adcs://", 7) == 0;
			MakeCall((isAdc ? "adch" : "nmdch"), "DataArrival", 1, h, line);
			handled = GetLuaBool();
		}
	}
	void onHub_OutgoingCommand(dcpp::interfaces::Hub* h, const char* line, bool& handled) throw() { 
		if(!handled) {
			handled = LuaManager::executeLuaCommand(line);
		}
	}
	void onHub_OutgoingMessage(dcpp::interfaces::Hub* h, dcpp::interfaces::ChatMessage* cm, bool& handled) throw() {
		if(!handled) {
			if(cm->getReplyTo() == 0) {
				// hub message
				MakeCall("dcpp", "OnCommandEnter", 1, h, (const char*)cm->getText());
				handled = GetLuaBool();
			} else {
				// private message
				/*dcppChatMessage* cm = (dcppChatMessage*)p1;
				if(!cm || !cm->replyTo) return DCPP_FALSE;
				//bool isAdc = strncmp(url, "adc://", 6) == 0 || strncmp(url, "adcs://", 7) == 0;
				dcppBuffer buf = { 0 };
				buf.buf = new char[40];
				buf.size = 40;
				memset(buf.buf, 0, (size_t)buf.size);

				LuaManager::dcppLib->call(DCPP_CALL_USER_GET_CID, cm->replyTo, (dcpp_param)&buf, 0);

				MakeCall("dcpp", cm->incoming ? "OnPrivateMessageIn" : "OnPrivateMessageOut", 1, (const char*)buf.buf, cm->message);

				delete buf.buf;
				return GetLuaBool() ? DCPP_TRUE : DCPP_FALSE;*/
			}
		}
	}

	void onHubManager_HubConnected(dcpp::interfaces::Hub* h) throw() { 
		bool isAdc = strncmp(h->getHubURL(), "adc://", 6) == 0 || strncmp(h->getHubURL(), "adcs://", 7) == 0;
		MakeCall((isAdc ? "adch" : "nmdch"), "OnHubAdded", 0, h);
		h->addEventListener((dcpp::interfaces::HubListener*)this);
	}

	void onHubManager_HubDisconnected(dcpp::interfaces::Hub* h) throw() { 
		bool isAdc = strncmp(h->getHubURL(), "adc://", 6) == 0 || strncmp(h->getHubURL(), "adcs://", 7) == 0;
		MakeCall((isAdc ? "adch" : "nmdch"), "OnHubRemoved", 0, h);
		h->remEventListener((dcpp::interfaces::HubListener*)this);
	}

	void onConnectionManager_ConnectionCreated(dcpp::interfaces::UserConnection* uc) throw() {
		uc->addEventListener((dcpp::interfaces::UserConnectionListener*)this);
	}
	void onConnectionManager_ConnectionDestroyed(dcpp::interfaces::UserConnection* uc) throw() {
		uc->remEventListener((dcpp::interfaces::UserConnectionListener*)this);
	}

	void onUserConnection_IncomingLine(dcpp::interfaces::UserConnection* uc, const char* line, bool& handled) throw() {
		if(!handled) {
			MakeCall("dcpp", "UserDataIn", 1, uc, line);
			handled = GetLuaBool();
		}
	}
	void onUserConnection_OutgoingLine(dcpp::interfaces::UserConnection* uc, const char* line, bool& handled) throw() {
		if(!handled) {
			MakeCall("dcpp", "UserDataOut", 1, uc, line);
			handled = GetLuaBool();
		}
	}
	void onUserConnection_AdcCommand(dcpp::interfaces::UserConnection* /*uc*/, dcpp::interfaces::AdcCommand* /*cmd*/) throw() {
	
	}

	void onTimer_Second(uint64_t) throw() { 
		if(LuaManager::timerActive) {
			MakeCall("dcpp", "OnTimer");
		}
	}
	void onTimer_Minute(uint64_t) throw() { 
		lua_gc(gL, LUA_GCCOLLECT, 0);
	}
};

Plugin* plugin = 0;
dcpp::interfaces::Core* core = 0;

extern "C" {
dcpp::interfaces::PluginInfo* __stdcall pluginInfo(unsigned long long coreSdkVersion, int svnRevision) {
    return &info;
}

int __stdcall pluginLoad(dcpp::interfaces::Core* c, void* pluginHandle) {
	core = c;
	plugin = new Plugin();
	return 0;
}

int __stdcall pluginUnload() {
	delete plugin;
	core = 0;
	return 0;
}

}
