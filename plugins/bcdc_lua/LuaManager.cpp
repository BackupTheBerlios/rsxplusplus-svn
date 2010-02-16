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
	{ "FindWindowHandle",		&LuaManager::not_implemented },
	{ "SendWindowMessage",		&LuaManager::not_implemented },
	{ "CreateClient",			&LuaManager::CreateClient },
	{ "DeleteClient",			&LuaManager::DeleteClient },
	{ "RunTimer",				&LuaManager::RunTimer },
	{ "GetSetting",				&LuaManager::GetSetting },
	{ "ToUtf8",					&LuaManager::ToUtf8 },
	{ "FromUtf8",				&LuaManager::FromUtf8 },
	{ "GetAppPath",				&LuaManager::GetAppPath },
	{ "DropUserConnection",		&LuaManager::DropUserConnection },
	{ 0 }
};

int LuaManager::DeleteClient(lua_State* L) {
	//@todo
	return 0;
}

int LuaManager::CreateClient(lua_State* L) {
	//@todo
	return 0;
}

int LuaManager::InjectHubMessage(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		//@todo - convert lua params
		LuaManager::dcppLib->call(DCPP_CALL_HUB_DISPATCH_LINE, (dcpp_ptr_t)lua_touserdata(L, -2), (dcpp_ptr_t)lua_tostring(L, -1), 0);
	}
	return 0;
}

int LuaManager::SendClientMessage(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		LuaManager::dcppLib->call(DCPP_CALL_CONNECTION_WRITE_LINE, (dcpp_ptr_t)lua_touserdata(L, -2), (dcpp_ptr_t)lua_tostring(L, -1), 0);
	}
	return 0;
}

int LuaManager::SendHubMessage(lua_State* L) {
	if(lua_gettop(L) == 2 && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		const char* msg = lua_tostring(L, -1);
		LuaManager::dcppLib->call(DCPP_CALL_HUB_SEND_USER_COMMAND, (dcpp_ptr_t)lua_touserdata(L, -2), (dcpp_ptr_t)msg, 0);
	}
	return 0;
}

int LuaManager::GenerateDebugMessage(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		dcppLib->call(DCPP_CALL_UTILS_LOG_MESSAGE, (dcpp_ptr_t)lua_tostring(L, -1), 0, 0);
	}
	return 0;
}

int LuaManager::SendUDPPacket(lua_State* L) {
	//@todo
	return 0;
}

int LuaManager::DropUserConnection(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_islightuserdata(L, -1)) {
		LuaManager::dcppLib->call(DCPP_CALL_CONNECTION_DISCONNECT, (dcpp_ptr_t)lua_touserdata(L, -1), 0, 0);
	}
	return 0;
}

int LuaManager::GetSetting(lua_State* L) {
	//@todo
	return 0;
}

int LuaManager::ToUtf8(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		const char* str = lua_tostring(L, -1);
		dcppBuffer buf;
		buf.size = (int)(2*strlen(str));
		buf.buf = new char[buf.size];
		memset(buf.buf, 0, buf.size);
		LuaManager::dcppLib->call(DCPP_CALL_UTILS_ACP_TO_UTF8, (dcpp_ptr_t)str, (dcpp_ptr_t)&buf, 0);
		lua_pushstring(L, buf.buf);
		delete[] buf.buf;
		return 1;
	} else {
		lua_pushliteral(L, "Missing argument");
		lua_error(L);
	}
	return 0;
}

int LuaManager::FromUtf8(lua_State* L) {
	if(lua_gettop(L) == 1 && lua_isstring(L, -1)) {
		const char* str = lua_tostring(L, -1);
		dcppBuffer buf;
		buf.size = (int)strlen(str);
		buf.buf = new char[buf.size];
		memset(buf.buf, 0, buf.size);
		LuaManager::dcppLib->call(DCPP_CALL_UTILS_UTF8_TO_ACP, (dcpp_ptr_t)str, (dcpp_ptr_t)&buf, 0);
		lua_pushstring(L, buf.buf);
		delete[] buf.buf;
		return 1;
	} else {
		lua_pushliteral(L, "Missing argument");
		lua_error(L);
	}
	return 0;
}

int LuaManager::GetAppPath(lua_State* L) {
	lua_pushstring(L, LuaManager::appPath);
	return 1;
}

int LuaManager::GetClientIp(lua_State* L) {
	dcpp_ptr_t connection = (dcpp_ptr_t)lua_touserdata(L, 1);
	if(connection > 0) {
		dcppConnectionInfo nfo;
		memset(&nfo, 0, sizeof(dcppConnectionInfo));
		if(LuaManager::dcppLib->call(DCPP_CALL_CONNECTION_GET_INFO, connection, (dcpp_ptr_t)&nfo, 0)) {
			lua_pushstring(L, nfo.ip);
			return 1;
		} else {
			lua_pushliteral(L, "GetClientIp: can't find connection at given address");
		}
	} else {
		lua_pushliteral(L, "GetClientIp: missing connection handle");
	}
	lua_error(L);
	return 0;
}

int LuaManager::GetHubIpPort(lua_State* L) {
	dcpp_ptr_t hub = (dcpp_ptr_t)lua_touserdata(L, 1);
	if(hub > 0) {
		dcppHubInfo nfo;
		memset(&nfo, 0, sizeof(dcppHubInfo));
		if(LuaManager::dcppLib->call(DCPP_CALL_HUB_GET_HUB_INFO, hub, (dcpp_ptr_t)&nfo, 0)) {
			lua_pushfstring(L, "%s:%d", nfo.ip, nfo.port);
			return 1;
		} else {
			lua_pushliteral(L, "GetHubIpPort: core can't find hub at given address");
		}
	} else {
		lua_pushliteral(L, "GetHubIpPort: missing hub handle");
	}
	lua_error(L);
	return 0;
}

int LuaManager::GetHubUrl(lua_State* L) {
	dcpp_ptr_t hub = (dcpp_ptr_t)lua_touserdata(L, 1);
	if(hub > 0) {
		dcppHubInfo nfo;
		memset(&nfo, 0, sizeof(dcppHubInfo));
		if(LuaManager::dcppLib->call(DCPP_CALL_HUB_GET_HUB_INFO, hub, (dcpp_ptr_t)&nfo, 0)) {
			lua_pushstring(L, nfo.url);
			return 1;
		} else {
			lua_pushliteral(L, "GetHubUrl: core can't find hub at given address");
		}
	} else {
		lua_pushliteral(L, "GetHubUrl: missing hub handle");
	}
	lua_error(L);
	return 0;
}

int LuaManager::RunTimer(lua_State* L) {
	//@todo
	return 0;
}
