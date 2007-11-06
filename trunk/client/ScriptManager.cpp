/*
 * Copyright (C) 2007 cologic, cologic@parsoma.net
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

#include "ScriptManager.h"
#include "Util.h"
#include "StringTokenizer.h"
#include "Client.h"
#include "ClientListener.h"
#include "UserConnection.h"
#include "ClientManager.h"
#include "DownloadManager.h"
#include "LogManager.h"
#include "NmdcHub.h"
#include "AdcHub.h"
#include "Thread.h"
#include <cstddef>
//yeah, kinda weird, but at least i can get funcs from winutil ;)
#include "../windows/stdafx.h"
#include "../windows/WinUtil.h"

ScriptManager* Singleton<ScriptManager>::instance = NULL;

static void callalert (lua_State *L, int status) {
	if (status != 0) {
		lua_getglobal(L, "_ALERT");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		}
		else {  /* no _ALERT function; print it on stderr */
			ScriptManager::getInstance()->SendDebugMessage(Text::acpToUtf8(string("LUA ERROR: ") + lua_tostring(L, -2)));
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

const char LuaManager::className[] = "DC";
Lunar<LuaManager>::RegType LuaManager::methods[] = {
	{"SendHubMessage", &LuaManager::SendHubMessage },
	{"SendClientMessage", &LuaManager::SendClientMessage },
	{"SendUDP", &LuaManager::SendUDPPacket},
	{"PrintDebug", &LuaManager::GenerateDebugMessage},
	{"GetClientIp", &LuaManager::GetClientIp},
	{"GetHubIpPort", &LuaManager::GetHubIpPort},
	{"GetHubUrl", &LuaManager::GetHubUrl},
	{"InjectHubMessage", &LuaManager::InjectHubMessageNMDC},
	{"InjectHubMessageADC", &LuaManager::InjectHubMessageADC},
	{"HubWindowAttention", &LuaManager::HubWindowAttention},
	{"FindWindowHandle", &LuaManager::FindWindow},
	{"SendWindowMessage", &LuaManager::SendMessage},
	{"CreateClient", &LuaManager::CreateClient},
	{"DeleteClient", &LuaManager::DeleteClient},
	{"RunTimer", &LuaManager::RunTimer},
	{"GetSetting", &LuaManager::GetSetting},
	{"ToUtf8", &LuaManager::ToUtf8},
	{"FromUtf8", &LuaManager::FromUtf8},
	{"GetAppPath", &LuaManager::GetAppPath},
	{"DropUserConnection", &LuaManager::DropUserConnection},
	//RSX++
	{"GetRSXSetting", &LuaManager::GetRSXSetting},
	{"MessageBox", &LuaManager::MessageBox},
	{"SendWindowMessageEx", &LuaManager::SendMessageEx},
	{"IsWindow", &LuaManager::IsWindow},
	{"OpenLink", &LuaManager::OpenLink},
	{"DecodeURI", &LuaManager::DecodeURI},
	//END
	{0}
};

int LuaManager::DeleteClient(lua_State* L){
	if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)){
		Client* client = (Client*) lua_touserdata(L, -1);
		ClientManager::getInstance()->putClient(client);
	}
	return 0;
}

int LuaManager::CreateClient(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)){
		Client* client = ClientManager::getInstance()->getClient(lua_tostring(L, -2));
		Identity ident;
		ident.setNick(lua_tostring(L, -1));
		client->setMyIdentity(ident);
		client->setPassword("");
		//this will block?
		client->connect();

		lua_pushlightuserdata(L, client);
		return 1;
	}

	return 0;
}

int LuaManager::InjectHubMessageNMDC(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1))
		reinterpret_cast<NmdcHub *>(lua_touserdata(L, -2))->onLine(lua_tostring(L, -1));

	return 0;
}

int LuaManager::InjectHubMessageADC(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1))
		reinterpret_cast<AdcHub *>(lua_touserdata(L, -2))->dispatch(lua_tostring(L, -1));

	return 0;
}

int LuaManager::HubWindowAttention(lua_State* L) {
	if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
		reinterpret_cast<Client *>(lua_touserdata(L, -1))->attention();
	}

	return 0;
}

int LuaManager::SendMessage(lua_State* L) {
	if (lua_gettop(L) == 4 && lua_islightuserdata(L, -4) && lua_isnumber(L, -3) &&
			lua_islightuserdata(L, -2) && lua_islightuserdata(L, -1)) {
		::SendMessage(reinterpret_cast<HWND>(lua_touserdata(L, -4)), static_cast<UINT>(lua_tonumber(L, -3)),
			reinterpret_cast<WPARAM>(lua_touserdata(L, -2)), reinterpret_cast<LPARAM>(lua_touserdata(L, -1)));
	}

	return 0;
}

int LuaManager::SendMessageEx(lua_State* L) {
	if (lua_gettop(L) == 4 && lua_islightuserdata(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
		::SendMessage(reinterpret_cast<HWND>(lua_touserdata(L, -4)), static_cast<UINT>(lua_tonumber(L, -3)),
			static_cast<WPARAM>(lua_tonumber(L, -2)), static_cast<LPARAM>(lua_tonumber(L, -1)));
	}
	return 0;
}

int LuaManager::FindWindow(lua_State* L) {
	//try to grab winamp handler with two strings in param....
	if(lua_gettop(L) == 2) {
		if(lua_isstring(L, -2) && (lua_isnumber(L, -1) && lua_tonumber(L, -1) == 0)) {
			lua_pushlightuserdata(L, ::FindWindow(Text::toT(string(lua_tostring(L, -2))).c_str(), NULL));
			return 1;
		} else if(lua_isstring(L, -2) && lua_isstring(L, -1)) {
			lua_pushlightuserdata(L, ::FindWindow(Text::toT(string(lua_tostring(L, -2))).c_str(), Text::toT(string(lua_tostring(L, -1))).c_str()));
			return 1;
		}
	}
	return 0;
}

int LuaManager::IsWindow(lua_State* L) {
	if(lua_gettop(L) == 1) {
		if(lua_islightuserdata(L, -1))
			return ::IsWindow(reinterpret_cast<HWND>(lua_touserdata(L, -1)));
	}
	return 0;
}

int LuaManager::SendClientMessage(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		reinterpret_cast<UserConnection *>(lua_touserdata(L, -2))->sendRaw(lua_tostring(L, -1));
	}

	return 0;
}

int LuaManager::SendHubMessage(lua_State* L) {
	if (lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		reinterpret_cast<Client*>(lua_touserdata(L, -2))->send(lua_tostring(L, -1));
	}

	return 0;
}

int LuaManager::GenerateDebugMessage(lua_State* L) {
	/* arguments: socket, buffer, address */
	if (lua_gettop(L) == 1 && lua_isstring(L, -1))
		ScriptManager::getInstance()->SendDebugMessage(lua_tostring(L, -1));

	return 0;
}

int LuaManager::SendUDPPacket(lua_State* L) {
	/* arguments: ip:port, data */
	if (lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		StringList sl = StringTokenizer<string>(lua_tostring(L, -2), ':').getTokens();
		ScriptManager::getInstance()->s.writeTo(sl[0], static_cast<short>(Util::toInt(sl[1])), lua_tostring(L, -1), lua_strlen(L, -1));
	}

	return 0;
}

int LuaManager::DropUserConnection(lua_State* L) {
	/* arguments: userconnection to drop */
	if (lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
		reinterpret_cast<UserConnection *>(lua_touserdata(L, -1))->disconnect();
	}

	return 0;
}

int LuaManager::GetSetting(lua_State* L) {
	/* arguments: string */
	int n, type;
	if(lua_gettop(L) == 1 && lua_isstring(L, -1) && SettingsManager::getInstance()->getType(lua_tostring(L, -1), n, type)) {
		if(type == SettingsManager::TYPE_STRING) {
			lua_pushstring(L, SettingsManager::getInstance()->get((SettingsManager::StrSetting)n).c_str());
			return 1;
		} else if(type == SettingsManager::TYPE_INT) {
			lua_pushnumber(L, SettingsManager::getInstance()->get((SettingsManager::IntSetting)n));
			return 1;
		} else if(type == SettingsManager::TYPE_INT64) {
			lua_pushnumber(L, static_cast<lua_Number>(SettingsManager::getInstance()->get((SettingsManager::Int64Setting)n)));
			return 1;
		}
	}
	lua_pushliteral(L, "GetSetting: setting not found");
	lua_error(L);
	return 0;
}
//RSX++
int LuaManager::GetRSXSetting(lua_State* L) {
	/* arguments: string */
	int n, type;
	if(lua_gettop(L) == 1 && lua_isstring(L, -1) && RSXSettingsManager::getInstance()->getType(lua_tostring(L, -1), n, type)) {
		if(type == RSXSettingsManager::TYPE_STRING) {
			lua_pushstring(L, RSXSettingsManager::getInstance()->get((RSXSettingsManager::StrSetting)n).c_str());
			return 1;
		} else if(type == RSXSettingsManager::TYPE_INT) {
			lua_pushnumber(L, RSXSettingsManager::getInstance()->get((RSXSettingsManager::IntSetting)n));
			return 1;
		} else if(type == RSXSettingsManager::TYPE_INT64) {
			lua_pushnumber(L, static_cast<lua_Number>(RSXSettingsManager::getInstance()->get((RSXSettingsManager::Int64Setting)n)));
			return 1;
		}
	}
	lua_pushliteral(L, "GetRSXSetting: setting not found");
	lua_error(L);
	return 0;
}

int LuaManager::MessageBox(lua_State* L) {
	if(lua_gettop(L) == 4 && lua_isstring(L, -1) && lua_isstring(L, -2) && lua_isstring(L, -3) && lua_isstring(L, -4)) {
		string icon = lua_tostring(L, -1);
		string buttons = lua_tostring(L, -2);
		UINT flags = 0;
		bool returnValue = false;

		if(icon == "error") {
			flags = MB_ICONHAND;
		} else if(icon == "question") {
			flags = MB_ICONQUESTION;
		} else if(icon == "warning") {
			flags = MB_ICONEXCLAMATION;
		} else if(icon == "info") {
			flags = MB_ICONASTERISK;
		} else if(icon == "info2") {
			flags = MB_ICONINFORMATION;
		}

		if(buttons == "ok") {
			flags += MB_OK;
		} else if(buttons == "yesno") {
			flags += MB_YESNO;
			returnValue = true;
		}

		if(returnValue) {
			if((::MessageBox(0, reinterpret_cast<LPCWSTR>(Text::toT(lua_tostring(L, -4)).c_str()), reinterpret_cast<LPCWSTR>(Text::toT(lua_tostring(L, -3)).c_str()), flags)) == IDYES)
				return 1;
		} else {
			::MessageBox(0, reinterpret_cast<LPCWSTR>(Text::toT(lua_tostring(L, -4)).c_str()), reinterpret_cast<LPCWSTR>(Text::toT(lua_tostring(L, -3)).c_str()), flags);
			return 0;
		}
	}
	return 0;
}

int LuaManager::OpenLink(lua_State *L) {
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		WinUtil::openLink(Text::toT(lua_tostring(L, -1)));
		return 1;
	}
	return 0;
}

int LuaManager::DecodeURI(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		lua_pushstring(L, Util::encodeURI(lua_tostring(L, -1)).c_str());
		return 1;
	}
	return 0;
}
//END
int LuaManager::ToUtf8(lua_State* L) {
	/* arguments: string */
	if(lua_gettop(L) == 1 && lua_isstring(L, -1) ) {
			lua_pushstring(L, Text::acpToUtf8(lua_tostring(L, -1)).c_str());
			return 1;
	} else {
		lua_pushliteral(L, "ToUtf8: string needed as argument");
		lua_error(L);
	}
	return 0;
}

int LuaManager::FromUtf8(lua_State* L) {
	/* arguments: string */
	if(lua_gettop(L) == 1 && lua_isstring(L, -1) ) {
			lua_pushstring(L, Text::utf8ToAcp(lua_tostring(L, -1)).c_str());
			return 1;
	} else {
		lua_pushliteral(L, "FromUtf8: string needed as argument");
		lua_error(L);
	}
	return 0;
}

int LuaManager::GetAppPath(lua_State* L) {
	lua_pushstring(L, Text::utf8ToAcp(Util::getDataPath()).c_str());
	return 1;
}

int LuaManager::GetClientIp(lua_State* L) {
	/* arguments: client */
	UserConnection* uc = (UserConnection*)lua_touserdata(L, 1);
	if(uc == NULL) {
		lua_pushliteral(L, "GetClientIpPort: missing client pointer");
		lua_error(L);
		return 0;
	}
	lua_pushstring(L, uc->getRemoteIp().c_str());
	return 1;
}

int LuaManager::GetHubIpPort(lua_State* L) {
	/* arguments: client */
	Client* c = (Client*)lua_touserdata(L, 1);
	if(c == NULL) {
		lua_pushliteral(L, "GetHubIpPort: missing hub pointer");
		lua_error(L);
		return 0;
	}
	lua_pushstring(L, c->getIpPort().c_str());
	return 1;
}

int LuaManager::GetHubUrl(lua_State* L) {
	/* arguments: client */
	Client* c = (Client*)lua_touserdata(L, 1);
	if(c == NULL) {
		lua_pushliteral(L, "GetHubUrl: missing hub pointer");
		lua_error(L);
		return 0;
	}
	lua_pushstring(L, c->getHubUrl().c_str());
	return 1;
}

int LuaManager::RunTimer(lua_State* L) {
	/* arguments: bool:on/off */
	if(lua_gettop(L) == 1 && lua_isnumber(L, -1)) {
		bool on = lua_tonumber(L, 1) != 0;	//shut VC++ up
		ScriptManager* sm = ScriptManager::getInstance();
		if(on != sm->getTimerEnabled()) {
			if(on)
				TimerManager::getInstance()->addListener(sm);
			else
				TimerManager::getInstance()->removeListener(sm);
			sm->setTimerEnabled(on);
		}
	} else {
		lua_pushliteral(L, "RunTimer: missing integer (0=off,!0=on)");
		lua_error(L);
		return 0;
	}
	return 1;
}

lua_State* ScriptInstance::L = 0;		//filled in by scriptmanager.
CriticalSection ScriptInstance::scs;

ScriptManager::ScriptManager() : isRunning(false), timerEnabled(false) {
}

void ScriptManager::load() {
	L = lua_open();
	luaL_openlibs(L);

	Lunar<LuaManager>::Register(L);
/*
 * let say we don't need formatting from lua, because we have it already hard coded in client
 * also, it don't want to work so leave it... ;D
 *
 * but it will work fine for OnColorize ;)
 */

	//create default text formatting function, in case startup.lua or formatting.lua isn't present.
	uint32_t color = SETTING(TEXT_COLOR);
	//this create a dcpp namespace. However, if startup.lua executes, if first clobbers this.
	string function =
			"dcpp = {_init_me_anyway = true}\n"
			"function dcpp.OnColorize(text)\n"
			"	text = string.gsub(text, \"([{}\\\\])\", \"\\%1\")\n"
			"	text = string.gsub(text, \"\\n\", \"\\\\line\\n\")\n"
			"	return \"{\\\\urtf1\\\\ansi\\\\ansicpg1252\\\\deff0\\\\plain0\\n\"..\n"
			"			\"{\\\\colortbl ;"
					"\\\\red" + Util::toString(color & 0xFF) +
					"\\\\green" + Util::toString((color >> 8) & 0xFF) +
					"\\\\blue" + Util::toString((color >> 16) & 0xFF) +
					";}\\n\"..\n"
			"			\"\\\\cf1 \"..text..\"}\\n\"\n"
			"end\n";
	lua_dostring(L, function.c_str());
	lua_pop(L, lua_gettop(L));		//hm. starts at 8 or so for me. I have no idea why...

	s.create(Socket::TYPE_UDP);

	ClientManager::getInstance()->addListener(this);
}

void ScriptInstance::EvaluateChunk(const string& chunk) {
	Lock l(scs);
	lua_dostring(L, chunk.c_str());
}

void ScriptInstance::EvaluateFile(const string& fn) {
	Lock l(scs);
	//RSX++ //remove log msg about missing file, probably user don't want use lua :]
	if(!Util::fileExists(Util::getDataPath() + "scripts\\" + fn))
		return;
	lua_dofile(L, (Util::getDataPath() + "scripts\\" + fn).c_str());
	ScriptManager::getInstance()->isRunning = true;
}

void ScriptManager::SendDebugMessage(const string &mess) {
	if(isRunning)
		LogManager::getInstance()->message(mess);
}

bool ScriptInstance::GetLuaBool() {
	//get value from top of stack, check if should cancel message.
	bool ret = false;
	if (lua_gettop(L) > 0) {
		ret = !lua_isnil(L, -1);
		lua_pop(L, 1);
	}
	return ret;
}

string ScriptInstance::GetClientType(const Client* aClient) {
//	return dynamic_cast<const AdcHub*>(const aClient) ? "adch" : "nmdch";
	if((strncmp(aClient->getHubUrl().c_str(), "adc://", 6) == 0) || (strncmp(aClient->getHubUrl().c_str(), "adcs://", 7) == 0)) {
		return "adch";
	} else {
		return "nmdch";
	}
}
//RSX++
string ScriptInstance::colorize(string& aLine) {
	Lock l(scs);
	MakeCall("dcpp", "OnColorize", 1, aLine);
	if(lua_isstring(L, -1))
		aLine = lua_tostring(L, -1);
	lua_settop(L, 0);
	return aLine;
}
//END
void ScriptManager::on(ClientDisconnected, const Client* aClient) throw() {
	MakeCall(GetClientType(aClient), "OnHubRemoved", 0, aClient);
}

void ScriptManager::on(ClientConnected, const Client* aClient) throw() {
	MakeCall(GetClientType(aClient), "OnHubAdded", 0, aClient);
}

void ScriptManager::on(Second, uint64_t /* ticks */) {
	MakeCall("dcpp", "OnTimer", 0, 0);
}

void ScriptInstance::LuaPush(int i) { lua_pushnumber(L, i); }
void ScriptInstance::LuaPush(const string& s) { lua_pushlstring(L, s.data(), s.size()); }

bool ScriptInstance::MakeCallRaw(const string& table, const string& method, int args, int ret) throw() {
	lua_getglobal(L, table.c_str());		// args + 1
	lua_pushstring(L, method.c_str());		// args + 2
	if (lua_istable(L, -2)) {
		lua_gettable(L, -2);				// args + 2
		lua_remove(L, -2);					// args + 1
		lua_insert(L, 1);					// args + 1
		if(lua_pcall(L, args, ret, 0) == 0) {
			dcassert(lua_gettop(L) == ret);
			return true;
		}
		const char *msg = lua_tostring(L, -1);
		string formatted_msg = (msg != NULL)?string("LUA Error: ") + msg:string("LUA Error: (unknown)");
		ScriptManager::getInstance()->SendDebugMessage(formatted_msg);
		dcassert(lua_gettop(L) == 1);
		lua_pop(L, 1);
	} else {
		lua_settop(L, 0);
	}
	return false;
}

/**
 * @file ScriptManager.cpp
 * $Id: ScriptManager.cpp,v 1.2 2004/06/19 12:57:22 cologic Exp $
 */
