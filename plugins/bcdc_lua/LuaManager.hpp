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

#ifndef LUA_MANAGER_HPP
#define LUA_MANAGER_HPP

#include "lua/src/luna.h"

#define TEMPBUF_SIZE 1024*8

extern int lua_dostring(const char* str);

struct LuaManager {
	//static dcppFunctions* dcppLib;
	static char appPath[MAX_PATH];
	static char configPath[MAX_PATH];
	static char tempBuffer[TEMPBUF_SIZE];
	static bool timerActive;

	static const char className[];
	static Luna<LuaManager>::RegType Register[];

	LuaManager(lua_State* /* L */) { }
	int SendClientMessage(lua_State* L);
	int SendHubMessage(lua_State* L);
	int GenerateDebugMessage(lua_State* L);
	int GetHubIpPort(lua_State* L);
	int GetHubUrl(lua_State* L);
	int GetClientIp(lua_State* L);
	int SendUDPPacket(lua_State* L);
	int InjectHubMessage(lua_State* L);
	int FindWindow(lua_State* L);
	int PostMessage(lua_State* L);
	int DropUserConnection(lua_State* L);

	int CreateClient(lua_State* L);
	int DeleteClient(lua_State* L);

	int RunTimer(lua_State* L);

	int GetSetting(lua_State* L);
	int SetSetting(lua_State* L);
	int GetAppPath(lua_State* L);
	int GetConfigPath(lua_State* L);

	int ToUtf8(lua_State* L);
	int FromUtf8(lua_State* L);

	int not_implemented(lua_State*) { return 0; };

	void raiseBadArgumentError(lua_State* L, const char* funcName, int exceptedArgCount);
	void raiseBadArgumentTypeError(lua_State* L, const char* funcName, int arg, const char* exceptedType, int argStackPos);
	
	static bool executeLuaCommand(const char* cmd);
	static void parseLuaCommand(const char* cmd, char*& out);
};

#endif
