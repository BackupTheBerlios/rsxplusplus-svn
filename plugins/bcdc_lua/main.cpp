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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdk/sdk.h"

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

			LuaManager::dcppLib->call(DCPP_CALL_UTILS_LOG_MESSAGE, (dcpp_param)message, 0, 0);

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
		LuaManager::dcppLib->call(DCPP_CALL_UTILS_LOG_MESSAGE, (dcpp_param)message, 0, 0);
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

dcppPluginInformation info = {
	"BCDC++ LuaScripts",
	"{338A05CC-B0A5-4533-9CF5-A7FD824AE6D2}",
	"adrian_007",
	"BCDC++ compatible " LUA_RELEASE " Interface",
	"http://rsxplusplus.sf.net",
	MAKE_VER(0, 0, 1, 0),
	SDK_VERSION
};

extern "C" {
dcppPluginInformation* DCPP_CALL_CONV pluginInfo(unsigned long long coreSdkVersion, int svnRevision) {
    return &info;
}

int DCPP_CALL_CONV onCoreLoad(int callReason, dcpp_param, dcpp_param, void*) {
	if(callReason == DCPP_EVENT_CORE_LOAD) {
		gL = luaL_newstate();
		luaL_openlibs(gL);

		Luna<LuaManager>::Register(gL);

		lua_pop(gL, lua_gettop(gL));

		dcppBuffer buf;
		buf.buf = new char[MAX_PATH+1];
		buf.size = MAX_PATH;
		memset(buf.buf, 0, (size_t)buf.size+1);

		size_t len;
		
		len = (size_t)LuaManager::dcppLib->call(DCPP_CALL_UTILS_GET_PATH, DCPP_UTILS_PATH_RESOURCES, (dcpp_param)&buf, 0);
		memcpy(LuaManager::appPath, buf.buf, len);
		memcpy(LuaManager::tempBuffer, buf.buf, len);
		strcat(LuaManager::tempBuffer, "scripts\\startup.lua");
		memset(buf.buf, 0, (size_t)buf.size);

		len = (size_t)LuaManager::dcppLib->call(DCPP_CALL_UTILS_GET_PATH, DCPP_UTILS_PATH_USER_CONFIG, (dcpp_param)&buf, 0);
		memcpy(LuaManager::configPath, buf.buf, len);

		lua_dofile(gL, LuaManager::tempBuffer);

		memset(LuaManager::tempBuffer, 0, TEMPBUF_SIZE);
		delete[] buf.buf;
	}
	return DCPP_FALSE;
}

int DCPP_CALL_CONV onHubEvent(int callReason, dcpp_param p1, dcpp_param p2, void*) {
	switch(callReason) {
		case DCPP_EVENT_HUB_CONNECTING: {
			const char* url = (const char*)p2;
			bool isAdc = strncmp(url, "adc://", 6) == 0 || strncmp(url, "adcs://", 7) == 0;
			MakeCall((isAdc ? "adch" : "nmdch"), "OnHubAdded", 0, p1);
			break;
		}
		case DCPP_EVENT_HUB_DESTROYED:
		case DCPP_EVENT_HUB_DISCONNECTED: {
			const char* url = (const char*)p2;
			bool isAdc = strncmp(url, "adc://", 6) == 0 || strncmp(url, "adcs://", 7) == 0;
			MakeCall((isAdc ? "adch" : "nmdch"), "OnHubRemoved", 0, p1);
			break;
		}
		case DCPP_EVENT_HUB_LINE: {
			dcppHubLine* line = (dcppHubLine*)p1;
			if(line) {
				if(line->incoming) {
					const char* url = (const char*)p2;
					bool isAdc = strncmp(url, "adc://", 6) == 0 || strncmp(url, "adcs://", 7) == 0;
					MakeCall((isAdc ? "adch" : "nmdch"), "DataArrival", 1, line->hubPtr, line->line);
					return GetLuaBool() ? DCPP_TRUE : DCPP_FALSE;
				} else {
					return LuaManager::executeLuaCommand(line->line) ? DCPP_TRUE : DCPP_FALSE;
				}
			}
			break;
		}
		case DCPP_EVENT_HUB_CHAT_SEND_LINE: {
			MakeCall("dcpp", "OnCommandEnter", 1, p1, (const char*)p2);
			return GetLuaBool() ? DCPP_TRUE : DCPP_FALSE;
			break;
		}
		case DCPP_EVENT_HUB_CHAT_MESSAGE: {
			dcppChatMessage* cm = (dcppChatMessage*)p1;
			if(!cm || !cm->replyTo) return DCPP_FALSE;
			//bool isAdc = strncmp(url, "adc://", 6) == 0 || strncmp(url, "adcs://", 7) == 0;
			dcppBuffer buf = { 0 };
			buf.buf = new char[40];
			buf.size = 40;
			memset(buf.buf, 0, (size_t)buf.size);

			LuaManager::dcppLib->call(DCPP_CALL_USER_GET_CID, cm->replyTo, (dcpp_param)&buf, 0);

			MakeCall("dcpp", cm->incoming ? "OnPrivateMessageIn" : "OnPrivateMessageOut", 1, (const char*)buf.buf, cm->message);

			delete buf.buf;
			return GetLuaBool() ? DCPP_TRUE : DCPP_FALSE;
		}
		default: {
			break;
		}
	}
	return DCPP_FALSE;
}

int DCPP_CALL_CONV onConnectionEvent(int callReason, dcpp_param p1, dcpp_param p2, void*) {
	switch(callReason) {
		case DCPP_EVENT_CONNECTION_LINE: {
			dcppConnectionLine* line = (dcppConnectionLine*)p1;
			if(line) {
				MakeCall("dcpp", p2 ? "UserDataIn" : "UserDataOut", 1, line->connectionPtr, line->line);
				return GetLuaBool() ? DCPP_TRUE : DCPP_FALSE;
			}
			break;
		}
		default: {
			break;
		}
	}
	return DCPP_FALSE;
}

int DCPP_CALL_CONV onTimer(int callReason, dcpp_param p1, dcpp_param p2, void*) {
	if(callReason == DCPP_EVENT_TIMER_TICK_SECOND && LuaManager::timerActive) {
		MakeCall("dcpp", "OnTimer");
	} else if(callReason == DCPP_EVENT_TIMER_TICK_MINUTE) {
		// collect every minute
		lua_gc(gL, LUA_GCCOLLECT, 0);
	}
	return DCPP_FALSE;
}

int DCPP_CALL_CONV pluginLoad(dcppFunctions* pF) {
	LuaManager::dcppLib = pF;
	LuaManager::dcppLib->addListener(DCPP_EVENT_CORE, onCoreLoad, 0);
	LuaManager::dcppLib->addListener(DCPP_EVENT_HUB, onHubEvent, 0);
	LuaManager::dcppLib->addListener(DCPP_EVENT_CONNECTION, onConnectionEvent, 0);
	LuaManager::dcppLib->addListener(DCPP_EVENT_TIMER, onTimer, 0);

	return 0;
}

int DCPP_CALL_CONV pluginUnload() {
	lua_close(gL);

	LuaManager::dcppLib->removeListener(DCPP_EVENT_CORE, onCoreLoad);
	LuaManager::dcppLib->removeListener(DCPP_EVENT_HUB, onHubEvent);
	LuaManager::dcppLib->removeListener(DCPP_EVENT_CONNECTION, onConnectionEvent);
	LuaManager::dcppLib->removeListener(DCPP_EVENT_TIMER, onTimer);

	LuaManager::dcppLib = 0;
	return 0;
}

}
