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

char LuaManager::appPath[MAX_PATH] = { 0 };
char LuaManager::configPath[MAX_PATH] = { 0 };
char LuaManager::tempBuffer[TEMPBUF_SIZE] = { 0 };
bool LuaManager::timerActive = false;

#define LOG(x) core->log(x)
#define LUA_ARGS_EQUAL(mL, x) (lua_gettop(mL) == x + 1)

const char LuaManager::className[] = "DC";
Luna<LuaManager>::RegType LuaManager::Register[] = {
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
	luaL_error(L, "Bad argument count in '%s' call (%d excepted, got %d)", funcName, exceptedArgCount, lua_gettop(L)-1);
}

void LuaManager::raiseBadArgumentTypeError(lua_State* L, const char* funcName, int arg, const char* exceptedType, int argStackPos) {
	luaL_error(L,"Bad argument #%d in '%s' call (%s excepted, got %s)", arg, funcName, exceptedType, lua_typename(L, lua_type(L, argStackPos)));
}

void LuaManager::parseLuaCommand(const char* cmd, char*& copy) {
	size_t len;
	char* pch;
	char* buffer;
	char* end;

	len = strlen(cmd);
	copy = (char*)malloc(len+11);
	memset(copy, 0, len+11);
	memcpy(copy, cmd, len);
	pch = copy;

	while((pch = strstr(pch, "%[lua:")) != 0) {
		memmove(pch+2, pch, strlen(pch));
		memcpy(pch, "<LUACMD>", 8);

		end = strstr(pch, "]");

		if(end) {
			buffer = pch;
			while((pch = strstr(pch, "!%")) != 0 && pch < end) {
				len = strlen(pch);
				memmove(pch, pch+1, len-1);
				pch[len-1] = 0;
				--end;
			}
			pch = buffer;
			while((pch = strstr(pch, "!{")) != 0 && pch < end) {
				len = strlen(pch);
				memmove(pch, pch+1, len-1);
				pch[0] = '[';
				pch[len-1] = 0;
				--end;
			}
			pch = buffer;
			while((pch = strstr(pch, "!}")) != 0 && pch < end) {
				len = strlen(pch);
				memmove(pch, pch+1, len-1);
				pch[0] = ']';
				pch[len-1] = 0;
				--end;
			}
			pch = end;
			memmove(pch+8, pch, strlen(pch));
			memcpy(pch, "</LUACMD>", 9);
			pch = end + 9;
		}
		len = strlen(copy);
		copy = (char*)realloc(copy, len+10);
		memset(copy+len, 0, 10);
		//LOG(copy);
	}
}

bool LuaManager::executeLuaCommand(const char* cmd) {
	size_t len;
	bool executed = false;
	char* pch = (char*)cmd;
	char* chunk;
	char* end;

	while((pch = (char*)strstr(pch, "<LUACMD>")) != 0) {
		pch += 8;
		end = strstr(pch, "</LUACMD>");

		if(end) {
			len = (size_t)(end-pch);
			chunk = new char[len+1];
			memset(chunk, 0, len+1);
			memcpy(chunk, pch, len);

			lua_dostring(chunk);

			delete[] chunk;
			pch = end + 9;
			executed = true;
		}
	}
	return executed;
}

int LuaManager::DeleteClient(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) && lua_islightuserdata(L, -1)) {
		//LuaManager::dcppLib->call(DCPP_CALL_HUB_CLOSE, (dcpp_param)lua_touserdata(L, -1), 0, 0);
	}
	return 0;
}

int LuaManager::CreateClient(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		//dcpp_param hub = LuaManager::dcppLib->call(DCPP_CALL_HUB_OPEN, (dcpp_param)lua_tostring(L, -2), 0, 0);
		//lua_pushlightuserdata(L, (void*)hub);
		//return 1;
	}
	return 0;
}

int LuaManager::InjectHubMessage(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		dcpp::interfaces::Hub* h = (dcpp::interfaces::Hub*)lua_touserdata(L, -2);
		if(h) {
			const char* line = lua_tostring(L, -1);
			h->parseLine(line);
		}
	}
	return 0;
}

int LuaManager::PostMessage(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 4) && lua_islightuserdata(L, -4) && lua_isnumber(L, -3) && lua_islightuserdata(L, -2) && lua_islightuserdata(L, -1)) {
		::SendMessage(
			reinterpret_cast<HWND>(lua_touserdata(L, -4)), 
			static_cast<UINT>(lua_tonumber(L, -3)),
			reinterpret_cast<WPARAM>(lua_touserdata(L, -2)), 
			reinterpret_cast<LPARAM>(lua_touserdata(L, -1)));
	}
	return 0;
}

int LuaManager::FindWindow(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		HWND hWindow = ::FindWindowA(lua_tostring(L, -2), lua_tostring(L, -1));
		lua_pushlightuserdata(L, hWindow);
		return 1;
	}
	return 0;
}

int LuaManager::SendClientMessage(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		dcpp::interfaces::UserConnection* uc = (dcpp::interfaces::UserConnection*)lua_touserdata(L, -2);
		if(uc) {
			const char* line = lua_tostring(L, -1);
			uc->sendLine(line);
		}
	}
	return 0;
}

int LuaManager::SendHubMessage(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) && lua_islightuserdata(L, -2) && lua_isstring(L, -1)) {
		dcpp::interfaces::Hub* h = (dcpp::interfaces::Hub*)lua_touserdata(L, -2);
		if(h) {
			const char* line = lua_tostring(L, -1);
			h->sendData(line, strlen(line));
		}
	}
	return 0;
}

int LuaManager::GenerateDebugMessage(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) && lua_isstring(L, -1)) {
		core->log(lua_tostring(L, -1));
	}
	return 0;
}

int LuaManager::SendUDPPacket(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		//const char* str = lua_tostring(L, -1);
		//LuaManager::dcppLib->call(DCPP_CALL_UTILS_SEND_UDP_PACKET, (dcpp_param)lua_tostring(L, -2), (dcpp_param)str, (dcpp_param)strlen(str));
	}
	return 0;
}

int LuaManager::DropUserConnection(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) && lua_islightuserdata(L, -1)) {
		dcpp::interfaces::UserConnection* uc = (dcpp::interfaces::UserConnection*)lua_touserdata(L, -2);
		if(uc) {
			uc->disconnect();
		} else {
			lua_pushliteral(L, "DropUserConnection: can't find connection at given address");
			lua_error(L);
		}
	}
	return 0;
}

int LuaManager::GetSetting(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) == false) {
		raiseBadArgumentError(L, "GetSetting", 2);
		return 0;
	}
	if(!lua_isstring(L, -2)) {
		raiseBadArgumentTypeError(L, "GetSetting", 1, "string", -2);
		return 0;
	}

	const char* key = lua_tostring(L, -2);

	int value;
	int64_t value64;
	const char* valueString;

	if(core->getCoreSetting(key, value)) {
		lua_pushnumber(L, value);
		return 1;
	} else if(core->getCoreSetting(key, value64)) {
		lua_pushnumber(L, static_cast<int>(value64));
		return 1;
	} else if(core->getCoreSetting(key, valueString)) {
		lua_pushstring(L, valueString);
		return 1;
	}

	luaL_error(L,"Bad argument #1 to 'GetSetting' (the given setting doesn't exists)");
	return 0;
}

int LuaManager::SetSetting(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 2) == false) {
		raiseBadArgumentError(L, "SetSetting", 2);
		return 0;
	}
	if(!lua_isstring(L, -2)) {
		raiseBadArgumentTypeError(L, "SetSetting", 1, "string", -2);
		return 0;
	}

	const char* key = lua_tostring(L, -2);

	if(lua_isnumber(L, -1)) {
		if(core->setCoreSetting(key, (int)lua_tonumber(L, -1)) == false && core->setCoreSetting(key, (int64_t)lua_tonumber(L, -1)) == false) {
			raiseBadArgumentTypeError(L, "SetSetting", 2, "setting does not exist", -1);
			return 0;
		}
		return 1;
	} else if(lua_isstring(L, -1)) {
		if(core->setCoreSetting(key, lua_tostring(L, -1)) == false) {
			raiseBadArgumentTypeError(L, "SetSetting", 2, "setting does not exist", -1);
			return 0;
		}
		return 1;
	} else {
		luaL_error(L,"Bad argument #1 to 'SetSetting' (bad param type)");
	}
	return 0;
}

int LuaManager::ToUtf8(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) == false) {
		raiseBadArgumentError(L, "ToUtf8", 1);
		return 0;
	} else if(!lua_isstring(L, -1)) {
		raiseBadArgumentTypeError(L, "ToUtf8", 1, "string", -1);
		return 0;
	}

	dcpp::interfaces::RefString str(core->getUtils()->convertFromAcpToUtf8(lua_tostring(L, -1)));
	lua_pushstring(L, str->get());
	return 1;
}

int LuaManager::FromUtf8(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) == false) {
		raiseBadArgumentError(L, "FromUtf8", 1);
		return 0;
	} else if(!lua_isstring(L, -1)) {
		raiseBadArgumentTypeError(L, "FromUtf8", 1, "string", -1);
		return 0;
	}

	dcpp::interfaces::RefString str(core->getUtils()->convertFromUtf8ToAcp(lua_tostring(L, -1)));
	lua_pushstring(L, str->get());
	return 1;
}

int LuaManager::GetAppPath(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 0) == false) {
		raiseBadArgumentError(L, "GetAppPath", 0);
		return 0;
	}

	lua_pushstring(L, LuaManager::appPath);
	return 1;
}

int LuaManager::GetConfigPath(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 0) == false) {
		raiseBadArgumentError(L, "GetConfigPath", 0);
		return 0;
	}

	lua_pushstring(L, LuaManager::configPath);
	return 1;
}

int LuaManager::GetClientIp(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) == false) {
		raiseBadArgumentError(L, "GetClientIp", 0);
		return 0;
	} else if(!lua_islightuserdata(L, -1)) {
		raiseBadArgumentTypeError(L, "GetClientIp", 1, "userdata", -1);
		return 0;
	}

	dcpp::interfaces::UserConnection* uc = (dcpp::interfaces::UserConnection*)lua_touserdata(L, -2);
	if(uc) {
		lua_pushstring(L, uc->getIp());
		return 1;
	} else {
		luaL_error(L, "GetClientIp: bad connection handle");
	}
	return 0;
}

int LuaManager::GetHubIpPort(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) == false) {
		raiseBadArgumentError(L, "GetHubIpPort", 0);
		return 0;
	} else if(!lua_islightuserdata(L, -1)) {
		raiseBadArgumentTypeError(L, "GetHubIpPort", 1, "userdata", -1);
		return 0;
	}

	dcpp::interfaces::Hub* h = (dcpp::interfaces::Hub*)lua_touserdata(L, -1);
	if(h) {
		lua_pushfstring(L, "%s:%d", h->getHubIP(), h->getPort());
		return 1;
	} else {
		luaL_error(L, "GetHubIpPort: bad hub handle");
	}
	return 0;
}

int LuaManager::GetHubUrl(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) == false) {
		raiseBadArgumentError(L, "GetHubUrl", 0);
		return 0;
	} else if(!lua_islightuserdata(L, -1)) {
		raiseBadArgumentTypeError(L, "GetHubUrl", 1, "userdata", -1);
		return 0;
	}

	dcpp::interfaces::Hub* h = (dcpp::interfaces::Hub*)lua_touserdata(L, -1);
	if(h) {
		lua_pushstring(L, h->getHubURL());
		return 1;
	} else {
		luaL_error(L, "GetHubUrl: bad hub handle");
	}
	return 0;
}

int LuaManager::RunTimer(lua_State* L) {
	if(LUA_ARGS_EQUAL(L, 1) == false) {
		raiseBadArgumentError(L, "RunTimer", 1);
		return 0;
	} else if(!lua_isnumber(L, -1)) {
		raiseBadArgumentTypeError(L, "RunTimer", 1, "number (4b)", -1);
		return 0;
	}

	LuaManager::timerActive = lua_tonumber(L, -1) != 0;
	return 0;
}
