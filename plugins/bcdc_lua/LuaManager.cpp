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

dcppFunctions* LuaManager::dcppLib = 0;
char LuaManager::appPath[MAX_PATH] = { 0 };
char LuaManager::configPath[MAX_PATH] = { 0 };
char LuaManager::tempBuffer[TEMPBUF_SIZE] = { 0 };
bool LuaManager::timerActive = false;

const char LuaManager::className[] = "DC";
Lunar<LuaManager>::RegType LuaManager::methods[] = {
	{ "SendHubMessage",			&LuaManager::SendHubMessage },
	{ "SendClientMessage",		&LuaManager::SendClientMessage },
	{ "SendUDP",				&LuaManager::SendUDPPacket },
	{ "PrintDebug",				&LuaManager::GenerateDebugMessage },
	{ "GetClientIp",			&LuaManager::GetClientIp },
	{ "GetHubIpPort",			&LuaManager::GetHubIpPort },
	{ "GetHubUrl",				&LuaManager::GetHubUrl },
	{ "InjectHubMessage",		&LuaManager::InjectHubMessage },
	{ "InjectHubMessageADC",	&LuaManager::InjectHubMessage },
	{ "HubWindowAttention",		&LuaManager::not_implemented },
	{ "FindWindowHandle",		&LuaManager::FindWindow },
	{ "SendWindowMessage",		&LuaManager::PostMessage },
	{ "CreateClient",			&LuaManager::CreateClient },
	{ "DeleteClient",			&LuaManager::DeleteClient },
	{ "RunTimer",				&LuaManager::RunTimer },
	{ "GetSetting",				&LuaManager::GetSetting },
	{ "SetSetting",				&LuaManager::SetSetting },
	{ "ToUtf8",					&LuaManager::ToUtf8 },
	{ "FromUtf8",				&LuaManager::FromUtf8 },
	{ "GetAppPath",				&LuaManager::GetAppPath },
	{ "GetConfigPath",			&LuaManager::GetConfigPath },
	{ "DropUserConnection",		&LuaManager::DropUserConnection },
	{ 0 }
};

void LuaManager::raiseBadArgumentError(lua_State* L, const char* funcName, int exceptedArgCount) {
	luaL_error(L, "Bad argument count in '%s' call (%d excepted, got %d)", funcName, exceptedArgCount, lua_gettop(L));
}

void LuaManager::raiseBadArgumentTypeError(lua_State* L, const char* funcName, int arg, const char* exceptedType, int argStackPos) {
	luaL_error(L,"Bad argument #%d in '%s' call (%s excepted, got %s)", arg, funcName, exceptedType, lua_typename(L, lua_type(L, argStackPos)));
}

int LuaManager::DeleteClient(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
		LuaManager::dcppLib->call(DCPP_CALL_HUB_CLOSE, (dcpp_param)lua_touserdata(L, -1), 0, 0);
	}
	return 0;
}

int LuaManager::CreateClient(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		dcpp_param hub = LuaManager::dcppLib->call(DCPP_CALL_HUB_OPEN, (dcpp_param)lua_tostring(L, -2), 0, 0);
		lua_pushlightuserdata(L, (void*)hub);
		return 1;
	}
	return 0;
}

int LuaManager::InjectHubMessage(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		//@todo - convert lua params
		LuaManager::dcppLib->call(DCPP_CALL_HUB_DISPATCH_LINE, (dcpp_param)lua_touserdata(L, -2), (dcpp_param)lua_tostring(L, -1), 0);
	}
	return 0;
}

int LuaManager::PostMessage(lua_State* L) {
	if(lua_gettop(L) == 4 && lua_islightuserdata(L, -4) && lua_isnumber(L, -3) && lua_islightuserdata(L, -2) && lua_islightuserdata(L, -1)) {
		::SendMessage(
			reinterpret_cast<HWND>(lua_touserdata(L, -4)), 
			static_cast<UINT>(lua_tonumber(L, -3)),
			reinterpret_cast<WPARAM>(lua_touserdata(L, -2)), 
			reinterpret_cast<LPARAM>(lua_touserdata(L, -1)));
	}
	return 0;
}

int LuaManager::FindWindow(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		HWND hWindow = ::FindWindowA(lua_tostring(L, -2), lua_tostring(L, -1));
		lua_pushlightuserdata(L, hWindow);
		return 1;
	}
	return 0;
}

int LuaManager::SendClientMessage(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		LuaManager::dcppLib->call(DCPP_CALL_CONNECTION_WRITE_LINE, (dcpp_param)lua_touserdata(L, -2), (dcpp_param)lua_tostring(L, -1), 0);
	}
	return 0;
}

int LuaManager::SendHubMessage(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		const char* msg = lua_tostring(L, -1);
		LuaManager::dcppLib->call(DCPP_CALL_HUB_SEND_USER_COMMAND, (dcpp_param)lua_touserdata(L, -2), (dcpp_param)msg, 0);
	}
	return 0;
}

int LuaManager::GenerateDebugMessage(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		dcppLib->call(DCPP_CALL_UTILS_LOG_MESSAGE, (dcpp_param)lua_tostring(L, -1), 0, 0);
	}
	return 0;
}

int LuaManager::SendUDPPacket(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		const char* str = lua_tostring(L, -1);
		LuaManager::dcppLib->call(DCPP_CALL_UTILS_SEND_UDP_PACKET, (dcpp_param)lua_tostring(L, -2), (dcpp_param)str, (dcpp_param)strlen(str));
	}
	return 0;
}

int LuaManager::DropUserConnection(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
		if(!LuaManager::dcppLib->call(DCPP_CALL_CONNECTION_DISCONNECT, (dcpp_param)lua_touserdata(L, -1), 0, 0)) {
			lua_pushliteral(L, "DropUserConnection: can't find connection at given address");
			lua_error(L);
		}
	}
	return 0;
}

int LuaManager::GetSetting(lua_State* L) {
	if(lua_gettop(L) != 2) {
		raiseBadArgumentError(L, "GetSetting", 2);
		return 0;
	}
	if(!lua_isstring(L, -2)) {
		raiseBadArgumentTypeError(L, "GetSetting", 1, "string", -2);
		return 0;
	}

	const char* key = lua_tostring(L, -2);
	dcpp_param settingValue;
	int type = (int)LuaManager::dcppLib->call(DCPP_CALL_CORE_SETTING_DCPP_GET, (dcpp_param)key, (dcpp_param)&settingValue, 0);

	switch(type) {
		case DCPP_SETTINGS_TYPE_INT: {
			if(!lua_isnumber(L, -1)) {
				raiseBadArgumentTypeError(L, "GetSetting", 2, "number (4b)", -1);
				return 0;
			}
			lua_pushnumber(L, static_cast<int>(settingValue));
			return 1;
		}
		case DCPP_SETTINGS_TYPE_INT64: {
			if(!lua_isnumber(L, -1)) {
				raiseBadArgumentTypeError(L, "GetSetting", 2, "number (8b)", -1);
				return 0;
			}
			lua_pushnumber(L, static_cast<lua_Number>(settingValue));
			return 1;
		}
		case DCPP_SETTINGS_TYPE_STRING: {
			if(!lua_isstring(L, -1)) {
				raiseBadArgumentTypeError(L, "GetSetting", 2, "string", -1);
				return 0;
			}
			lua_pushstring(L, reinterpret_cast<const char*>(settingValue));
			return 1;
		}
		default: {
			luaL_error(L,"Bad argument #1 to 'GetSetting' (the given setting doesn't exists)");
		}
	}
	return 0;
}

int LuaManager::SetSetting(lua_State* L) {
	if(lua_gettop(L) != 2) {
		raiseBadArgumentError(L, "SetSetting", 2);
		return 0;
	}
	if(!lua_isstring(L, -2)) {
		raiseBadArgumentTypeError(L, "SetSetting", 1, "string", -2);
		return 0;
	}

	const char* key = lua_tostring(L, -2);
	int type = (int)LuaManager::dcppLib->call(DCPP_CALL_CORE_SETTING_DCPP_GET_TYPE, (dcpp_param)key, 0, 0);

	switch(type) {
		case DCPP_SETTINGS_TYPE_INT: {
			if(!lua_isnumber(L, -1)) {
				raiseBadArgumentTypeError(L, "SetSetting", 2, "number (4b)", -1);
				return 0;
			}
			LuaManager::dcppLib->call(DCPP_CALL_CORE_SETTING_DCPP_SET,(dcpp_param)key,(dcpp_param)lua_tonumber(L, -1), 0);
			return 1;
		}
		case DCPP_SETTINGS_TYPE_INT64: {
			if(!lua_isnumber(L, -1)) {
				raiseBadArgumentTypeError(L, "SetSetting", 2, "number (8b)", -1);
				return 0;
			}
			LuaManager::dcppLib->call(DCPP_CALL_CORE_SETTING_DCPP_SET,(dcpp_param)key,(dcpp_param)lua_tonumber(L, -1), 0);
			return 1;
		}
		case DCPP_SETTINGS_TYPE_STRING: {
			if(!lua_isstring(L, -1)) {
				raiseBadArgumentTypeError(L, "SetSetting", 2, "string", -1);
				return 0;
			}
			LuaManager::dcppLib->call(DCPP_CALL_CORE_SETTING_DCPP_SET,(dcpp_param)key,(dcpp_param)lua_tostring(L, -1), 0);
			return 1;
		}
		default: {
			luaL_error(L,"Bad argument #1 to 'SetSetting' (the given setting doesn't exists)");
		}
	}
	return 0;
}

int LuaManager::ToUtf8(lua_State* L) {
	if(lua_gettop(L) != 1) {
		raiseBadArgumentError(L, "ToUtf8", 1);
		return 0;
	} else if(!lua_isstring(L, -1)) {
		raiseBadArgumentTypeError(L, "ToUtf8", 1, "string", -1);
		return 0;
	}

	const char* str = lua_tostring(L, -1);
	dcppBuffer buf;
	buf.size = (int)(2*strlen(str));
	buf.buf = new char[buf.size];
	memset(buf.buf, 0, buf.size);
	LuaManager::dcppLib->call(DCPP_CALL_UTILS_ACP_TO_UTF8, (dcpp_param)str, (dcpp_param)&buf, 0);
	lua_pushstring(L, buf.buf);
	delete[] buf.buf;
	return 1;
}

int LuaManager::FromUtf8(lua_State* L) {
	if(lua_gettop(L) != 1) {
		raiseBadArgumentError(L, "FromUtf8", 1);
		return 0;
	} else if(!lua_isstring(L, -1)) {
		raiseBadArgumentTypeError(L, "FromUtf8", 1, "string", -1);
		return 0;
	}

	const char* str = lua_tostring(L, -1);
	dcppBuffer buf;
	buf.size = (int)strlen(str);
	buf.buf = new char[buf.size];
	memset(buf.buf, 0, buf.size);
	LuaManager::dcppLib->call(DCPP_CALL_UTILS_UTF8_TO_ACP, (dcpp_param)str, (dcpp_param)&buf, 0);
	lua_pushstring(L, buf.buf);
	delete[] buf.buf;
	return 1;
}

int LuaManager::GetAppPath(lua_State* L) {
	if(lua_gettop(L) != 0) {
		raiseBadArgumentError(L, "GetAppPath", 0);
		return 0;
	}

	lua_pushstring(L, LuaManager::appPath);
	return 1;
}

int LuaManager::GetConfigPath(lua_State* L) {
	if(lua_gettop(L) != 0) {
		raiseBadArgumentError(L, "GetConfigPath", 0);
		return 0;
	}

	lua_pushstring(L, LuaManager::configPath);
	return 1;
}

int LuaManager::GetClientIp(lua_State* L) {
	if(lua_gettop(L) != 1) {
		raiseBadArgumentError(L, "GetClientIp", 0);
		return 0;
	} else if(!lua_islightuserdata(L, -1)) {
		raiseBadArgumentTypeError(L, "GetClientIp", 1, "userdata", -1);
		return 0;
	}

	dcpp_param connection = (dcpp_param)lua_touserdata(L, -1);
	if(connection > 0) {
		dcppConnectionInfo nfo;
		memset(&nfo, 0, sizeof(dcppConnectionInfo));
		if(LuaManager::dcppLib->call(DCPP_CALL_CONNECTION_GET_INFO, connection, (dcpp_param)&nfo, 0)) {
			lua_pushstring(L, nfo.ip);
			return 1;
		} else {
			luaL_error(L, "GetClientIp: can't find connection at given address");
		}
	} else {
		luaL_error(L, "GetClientIp: bad connection handle");
	}
	return 0;
}

int LuaManager::GetHubIpPort(lua_State* L) {
	if(lua_gettop(L) != 1) {
		raiseBadArgumentError(L, "GetHubIpPort", 0);
		return 0;
	} else if(!lua_islightuserdata(L, -1)) {
		raiseBadArgumentTypeError(L, "GetHubIpPort", 1, "userdata", -1);
		return 0;
	}

	dcpp_param hub = (dcpp_param)lua_touserdata(L, -1);
	if(hub > 0) {
		dcppHubInfo nfo;
		memset(&nfo, 0, sizeof(dcppHubInfo));
		if(LuaManager::dcppLib->call(DCPP_CALL_HUB_GET_HUB_INFO, hub, (dcpp_param)&nfo, 0)) {
			lua_pushfstring(L, "%s:%d", nfo.ip, nfo.port);
			return 1;
		} else {
			luaL_error(L, "GetHubIpPort: core can't find hub at given address");
		}
	} else {
		luaL_error(L, "GetHubIpPort: bad hub handle");
	}
	return 0;
}

int LuaManager::GetHubUrl(lua_State* L) {
	if(lua_gettop(L) != 1) {
		raiseBadArgumentError(L, "GetHubUrl", 0);
		return 0;
	} else if(!lua_islightuserdata(L, -1)) {
		raiseBadArgumentTypeError(L, "GetHubUrl", 1, "userdata", -1);
		return 0;
	}

	dcpp_param hub = (dcpp_param)lua_touserdata(L, 1);
	if(hub > 0) {
		dcppHubInfo nfo;
		memset(&nfo, 0, sizeof(dcppHubInfo));
		if(LuaManager::dcppLib->call(DCPP_CALL_HUB_GET_HUB_INFO, hub, (dcpp_param)&nfo, 0)) {
			lua_pushstring(L, nfo.url);
			return 1;
		} else {
			luaL_error(L, "GetHubUrl: core can't find hub at given address");
		}
	} else {
		luaL_error(L, "GetHubUrl: bad hub handle");
	}
	return 0;
}

int LuaManager::RunTimer(lua_State* L) {
	if(lua_gettop(L) != 1) {
		raiseBadArgumentError(L, "RunTimer", 0);
		return 0;
	} else if(!lua_isnumber(L, -1)) {
		raiseBadArgumentTypeError(L, "RunTimer", 1, "number (4b)", -1);
		return 0;
	}

	LuaManager::timerActive = lua_tonumber(L, -1) != 0;
	return 0;
}
