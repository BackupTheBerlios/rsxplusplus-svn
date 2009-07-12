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

#ifndef RSXPLUSPLUS_LUA_BINDINGS
#define RSXPLUSPLUS_LUA_BINDINGS

extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <bit.h>
}

#include <luabind/luabind.hpp>

namespace dcpp {

	namespace LuaBindings {
		void BindClient(lua_State* L);
		void BindUser(lua_State* L);
		void BindOnlineUser(lua_State* L);
		void BindIdentity(lua_State* L);
		void BindCID(lua_State* L);
		void BindAdcCommand(lua_State* L);
		void BindUserConnection(lua_State* L);
		void BindFlags(lua_State* L);
		void BindSimpleXML(lua_State* L);
		void BindDcppCore(lua_State* L);
		void BindShareManager(lua_State* L);
		void BindConnectionManager(lua_State* L);
		void BindQueueManager(lua_State* L);
		void BindTTHValue(lua_State* L);
	}
}

#endif