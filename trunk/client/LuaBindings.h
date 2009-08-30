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
#include <luabind/object.hpp>

// we need custom converter, since lua stores all values as double afaik...
//@todo better way to keep precision?

namespace luabind {
	template <>
	struct default_converter<int64_t> : native_converter_base<int64_t> {
      static int compute_score(lua_State* L, int index) {
          return default_converter<int>::compute_score(L, index);
      }

      int64_t from(lua_State* L, int index) {
          return (int64_t)lua_tonumber(L, index);
      }

      void to(lua_State* L, int64_t value) {
          lua_pushnumber(L, value);
      }
	};

	template <>
	struct default_converter<uint64_t> : native_converter_base<uint64_t> {
      static int compute_score(lua_State* L, int index) {
          return default_converter<int>::compute_score(L, index);
      }

      uint64_t from(lua_State* L, int index) {
          return (uint64_t)lua_tonumber(L, index);
      }

      void to(lua_State* L, uint64_t value) {
          lua_pushnumber(L, value);
      }
	};
}

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
		void BindSettingsManager(lua_State* L);
		void BindClientManager(lua_State* L);
		void BindFavoriteManager(lua_State* L);
	}
}

#endif
