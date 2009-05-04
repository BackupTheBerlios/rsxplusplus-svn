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
#include "LuaBindings.h"

#include <shellapi.h>

#include <luabind/dependency_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/raw_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <boost/ref.hpp>

#include "LogManager.h"
#include "Client.h"
#include "User.h"
#include "UserConnection.h"
#include "rsxppSettingsManager.h"
#include "ScriptManager.h"
#include "SimpleXML.h"
#include "AdcCommand.h"
#include "version.h"

namespace dcpp {
	namespace wrappers {
		inline void LogMessage(const std::string& msg) {
			LogManager::getInstance()->message(msg);
		}
		inline HWND findWindowA(LPCSTR p1, LPCSTR p2) {
			return FindWindowA(strcmp(p1, "") == 0 ? NULL : p1, strcmp(p2, "") == 0 ? NULL : p2);
		}
		inline LRESULT sendMessageA(HWND p1, UINT p2, LPARAM p3, WPARAM p4) {
			return SendMessageA(p1, p2, p3, p4);
		}
		inline LRESULT postMessageA(HWND p1, UINT p2, LPARAM p3, WPARAM p4) {
			return PostMessageA(p1, p2, p3, p4);
		}
		inline string getWindowTextA(HWND p1, int len) {
			string buf;
			buf.resize(len + 1);
			::GetWindowTextA(p1, &buf[0], buf.size());
			return buf;
		}
		inline bool setWindowTextA(HWND p1, LPCSTR p2) {
			return ::SetWindowTextA(p1, p2) ? true : false;
		}
		inline int getWindowTextLengthA(HWND p1) {
			return ::GetWindowTextLengthA(p1);
		}
		inline HINSTANCE shellExecuteA(HWND p1, LPCSTR p2, LPCSTR p3, LPCSTR p4, LPCSTR p5, int p6) {
			return ShellExecuteA(p1, p2, p3, p4, p5, p6);
		}
		inline void privateMessage(Client* c, OnlineUser* ou, const std::string& msg, bool thirdPerson) {
			if(c && ou)
				c->privateMessage(ou, msg, thirdPerson);
		}
		inline string makeParams(const luabind::object& o, const string& format) {
			if(luabind::type(o) == LUA_TTABLE) {
				StringMap params;
				luabind::iterator i(o), end;
				for( ; i != end; ++i )
					params[luabind::object_cast<std::string>(i.key())] = luabind::object_cast<std::string>(*i);
				return Util::formatParams(format, params, false);
			}
			return Util::emptyString;
		}
		luabind::object getUsers(lua_State* L, Client* c) {
			luabind::object o = luabind::newtable(L);
			OnlineUserList list;
			c->getUserList(list);
			for(OnlineUserList::iterator i = list.begin(); i != list.end(); ++i) {
				OnlineUser* u = (*i).get();
				o[u->getIdentity().getNick()] = u;
			}
			return o;
		}
		inline double getVersion(int type) {
			switch(type) {
				case 1: return VERSIONFLOAT;
				case 2: return DCVERSIONFLOAT;
				case 3: return SVN_REVISION;
			}
			return 0.0;
		}
		inline User* getUser(OnlineUser* ou) {
			return ou->getUser().get();
		}
		inline User* getUser2(Identity* i) {
			return i->getUser().get();
		}
		inline string toBase32(CID* cid) {
			const uint8_t* data = cid->data();
			return Encoder::toBase32(data, sizeof(data));
		}
		inline void sendAction(Client* c, OnlineUser* o, int id) {
			c->sendActionCommand(*o, id);
		}
	}

	namespace LuaBindings {
		void BindClient(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<Client>("Client")
				.def("hubMessage", &Client::hubMessage)
				.def("privateMessage", &wrappers::privateMessage)
				.def("sendUserCmd", &Client::sendUserCmd)
				.def("getUserCount", &Client::getUserCount)
				.def("getAvailable", &Client::getAvailable)
				.def("isConnected", &Client::isConnected)
				.def("isOp", &Client::isOp)
				.def("isActive", &Client::isActive)
				.def("getAddress", &Client::getAddress)
				.def("getPort", &Client::getPort)
				.def("getIp", &Client::getIp)
				.def("getLocalIp", &Client::getLocalIp)
				.def("addHubLine", &Client::addHubLine)
				.def("cheatMessage", &Client::cheatMessage)
				.def("getMyNick", &Client::getMyNick)
				.def("getHubName", &Client::getHubName)
				.def("getHubDescription", &Client::getHubDescription)
				.def("getHubUrl", &Client::getHubUrl)
				.def("escape", &Client::escape)
				.def("sendAction", &wrappers::sendAction)
				.property("password", &Client::getPassword, &Client::setPassword)
				.property("currentNick", &Client::getCurrentNick, &Client::setCurrentNick)
				.property("stealth", &Client::getStealth, &Client::setStealth)
				.def("getMyIdentity", (Identity& (Client::*)())&Client::getMyIdentity, luabind::dependency(luabind::result, _1))
				.def("getHubIdentity", (Identity& (Client::*)())&Client::getHubIdentity, luabind::dependency(luabind::result, _1))
				.def("findUser", &Client::findOnlineUser)
				.def("getUserList", &wrappers::getUsers, luabind::raw(_1))
			];
		}

		void BindUser(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<User, Flags>("User")
				.def("getCID", &User::getCID)
				.def("isOnline", &User::isOnline)
				.def("isNMDC", &User::isNMDC)
				.def("inc", &User::inc)
				.def("dec", &User::dec)
				.def("unique", &User::unique)
			];
		}

		void BindOnlineUser(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<OnlineUser>("OnlineUser")
				.def("getIdentity", (Identity& (OnlineUser::*)())&OnlineUser::getIdentity, luabind::dependency(luabind::result, _1))
				.def("getUser", &wrappers::getUser, luabind::dependency(luabind::result, _1))
				.def("getClient", (Client& (OnlineUser::*)())&OnlineUser::getClient, luabind::dependency(luabind::result, _1))
				.def("inc", &OnlineUser::inc)
				.def("dec", &OnlineUser::dec)
				.def("unique", &OnlineUser::unique)
			];
		}

		void BindIdentity(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<Identity>("Identity")
				.def("get", &Identity::get)
				.def("set", &Identity::set)
				.def("isSet", &Identity::isSet)
				.def("supports", &Identity::supports)
				.def("isClientType", &Identity::isClientType)
				.def("getUser", &wrappers::getUser2, luabind::dependency(luabind::result, _1))
				.def("getSIDString", &Identity::getSIDString)
			];
		}

		void BindCID(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<CID>("CID")
				.def("toBase32", &wrappers::toBase32)
				.def("toHash", &CID::toHash)
				.def("isZero",  &CID::isZero)
			];
		}

		void BindAdcCommand(lua_State* /*L*/) {
			/*luabind::module(L, "dcpp") [
				luabind::class_<AdcCommand>("AdcCommand")
				.def(luabind::constructor<uint32_t, char>())
				.def(luabind::constructor<uint32_t, const uint32_t, char>())
			];*/
		}

		void BindUserConnection(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<UserConnection, Flags>("UserConnection")
				.def("getRemoteIp", &UserConnection::getRemoteIp)
				.def("getPort", &UserConnection::getPort)
				.def("getHubUrl", &UserConnection::getHubUrl)
				.def("isSecure", &UserConnection::isSecure)
				.def("isTrusted", &UserConnection::isTrusted)
				.def("getCipherName", &UserConnection::getCipherName)
				.def("send", &UserConnection::sendRaw)
				.def("updated", &UserConnection::updated)
				.def("disconnect", &UserConnection::disconnect)
				//.enum_("UcFlags") [
				//	luabind::value("FLAG_NMDC", UserConnection::FLAG_NMDC),
				//	luabind::value("FLAG_OP", UserConnection::FLAG_OP)
				//]
			];
		}

		void BindFlags(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<Flags>("Flags")
				.def("isSet", &Flags::isSet)
				.def("isAnySet", &Flags::isAnySet)
				.def("setFlag", &Flags::setFlag)
				.def("unsetFlag", &Flags::unsetFlag)
			];
		}

		void BindSimpleXML(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<SimpleXML>("SimpleXML")
				.def(luabind::constructor<>())
				.def("addTag", (void (SimpleXML::*)(const string&, const string&))&SimpleXML::addTag)
				.def("addAttrib", (void (SimpleXML::*)(const string&, const string&))&SimpleXML::addAttrib)
				.def("addChildAttrib", (void (SimpleXML::*)(const string&, const string&))&SimpleXML::addChildAttrib)
				.def("getData", &SimpleXML::getData)
				.def("stepIn", &SimpleXML::stepIn)
				.def("stepOut", &SimpleXML::stepOut)
				.def("resetCurrentChild", &SimpleXML::resetCurrentChild)
				.def("findChild", &SimpleXML::findChild)
				.def("getChildData", &SimpleXML::getChildData)
				.def("getChildAttrib", &SimpleXML::getChildAttrib)
				.def("fromXML", &SimpleXML::fromXML)
				.def("toXML", (string (SimpleXML::*)())&SimpleXML::toXML)
			];
		}

		void BindDcppCore(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::def("LogMessage", &wrappers::LogMessage),
				luabind::def("TextToUTF8", (string (*)(const string&))&Text::acpToUtf8),
				luabind::def("EncodeURI", &Util::encodeURI),
				luabind::def("formatParams", &wrappers::makeParams),
				luabind::def("getVersion", &wrappers::getVersion)
			];
		}

		// deprecated
		void BindWinAPI(lua_State* L) {
			luabind::module(L, "winapi") [
				luabind::class_<HWND__>("HWND"),
				luabind::class_<HINSTANCE__>("HINSTANCE"),

				luabind::def("SendMessageA", &wrappers::sendMessageA),
				luabind::def("PosMessageA", &wrappers::postMessageA),
				luabind::def("FindWindowA", &wrappers::findWindowA),
				luabind::def("GetWindowTextA", &wrappers::getWindowTextA),
				luabind::def("SetWindowTextA", &wrappers::setWindowTextA),
				luabind::def("GetWindowTextLengthA", &wrappers::getWindowTextLengthA),
				luabind::def("ShellExecuteA", &wrappers::shellExecuteA)
			];
		}

	}
}