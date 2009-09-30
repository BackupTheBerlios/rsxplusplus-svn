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

#include <luabind/dependency_policy.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/raw_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/discard_result_policy.hpp>
#include <boost/ref.hpp>

#include "LogManager.h"
#include "Client.h"
#include "User.h"
#include "ConnectionManager.h"
#include "UserConnection.h"
#include "rsxppSettingsManager.h"
#include "ScriptManager.h"
#include "SimpleXML.h"
#include "AdcCommand.h"
#include "version.h"
#include "ShareManager.h"
#include "QueueManager.h"
#include "FavoriteManager.h"

namespace dcpp {

	namespace wrappers {
		inline void LogMessage(const std::string& msg) {
			LogManager::getInstance()->message(msg);
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

		void getUsers(Client* c, const luabind::object& table) {
			if(luabind::type(table) == LUA_TTABLE) {
				OnlineUserList list;
				c->getUserList(list);
				for(OnlineUserList::iterator i = list.begin(); i != list.end(); ++i) {
					table[(*i)->getIdentity().getNick()] = (*i).get();
				}
			}
		}

		void getHubs(ClientManager* cm, const luabind::object& table) {
			if(luabind::type(table) == LUA_TTABLE) {
				const Client::List& l = cm->getClients();
				for(Client::List::const_iterator i = l.begin(); i != l.end(); ++i) {
					table[*(i->first)] = i->second;
				}
			}
		}

		void getOnlineUsers(ClientManager* cm, const luabind::object& table) {
			if(luabind::type(table) == LUA_TTABLE) {
				const ClientManager::OnlineMap& users = cm->getOnlineUsers();
				for(ClientManager::OnlineIterC i = users.begin(); i != users.end(); ++i) {
					table[i->first] = i->second;
				}
			}
		}

		void getIdentityParams(Identity& usr, const luabind::object& table, const std::string& prefix) {
			if(luabind::type(table) == LUA_TTABLE) {
				StringMap p;
				usr.getParams(p, prefix, false);
				for(StringMap::const_iterator i = p.begin(); i != p.end(); ++i) {
					table[i->first] = i->second;
				}
			}
		}

		inline double getVersion(int type) {
			switch(type) {
				case 1: return VERSIONFLOAT;
				case 2: return DCVERSIONFLOAT;
				case 3: return SVN_REVISION;
				case 4: 
#ifdef _WIN64
					return 64.0;
#else
					return 32.0;
#endif
				case 5:
#ifdef _DEBUG
					return 3;
#elif SVNBUILD
					return 2;
#else
					return 1;
#endif
			}
			return 0.0;
		}
		template<typename T>
		inline User* getUser(T* u) {
			return u->getUser().get();
		}
		inline void sendAction(Client* c, OnlineUser* o, int id) {
			c->sendActionCommand(*o, id);
		}
		inline void OnlineUserConnect(OnlineUser* ou, const string& token) {
			ou->getClient().connect(*ou, token);
		}
		inline void getDownloadConnection(ConnectionManager* cm, User* u, const string& hubUrl) {
			cm->getDownloadConnection(u, hubUrl);
		}
		inline bool QueueManager_addFile(QueueManager* q, const string& aTarget, int64_t aSize, const string& root, User* aUser, const string& hubHint, int aFlags, bool addBad) {
			try {
				q->add(aTarget, aSize, TTHValue(root), aUser, hubHint, (Flags::MaskType)aFlags, addBad);
				return true;
			} catch(...) { }
			return false;
		}
		inline string getSystemCharset() {
			return Text::systemCharset;
		}
		inline bool isAdcHub(Client* c) {
			return strnicmp("adc://", c->getHubUrl().c_str(), 6) == 0 || strnicmp("adcs://", c->getHubUrl().c_str(), 7) == 0;
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
				.def("getHubName", &Client::getHubName)
				.def("getHubDescription", &Client::getHubDescription)
				.def("getHubUrl", &Client::getHubUrl)
				.def("escape", &Client::escape)
				.def("sendAction", &wrappers::sendAction)
				.def("redirect", &Client::redirect)
				.property("password", &Client::getPassword, &Client::setPassword)
				.property("currentNick", &Client::getCurrentNick, &Client::setCurrentNick)
				.property("stealth", &Client::getStealth, &Client::setStealth)
				.def("getMyIdentity", (Identity& (Client::*)())&Client::getMyIdentity, luabind::dependency(luabind::result, _1))
				.def("getHubIdentity", (Identity& (Client::*)())&Client::getHubIdentity, luabind::dependency(luabind::result, _1))
				.def("findUser", (OnlineUser* (Client::*)(const string&) const)&Client::findOnlineUser)
				.def("findUser", (OnlineUser* (Client::*)(const CID&) const)&Client::findUser)
				.def("findUser", (OnlineUser* (Client::*)(const uint32_t) const)&Client::findUser)
				.def("getUserList", &wrappers::getUsers)
				.def("parseCommand", &Client::parseCommand)
				.def("info", &Client::info)
				.def("isADC", &wrappers::isAdcHub)
				//.def("sendAdcCommand", (void (Client::*)(const AdcCommand&))&Client::send, luabind::adopt(luabind::result))
				.enum_("MessageStyle") [
					luabind::value("STYLE_GENERAL", 0),
					luabind::value("STYLE_MY_OWN", 1),
					luabind::value("STYLE_SERVER", 2),
					luabind::value("STYLE_SYSTEM", 3),
					luabind::value("STYLE_LOG", 4),
					luabind::value("STYLE_CHEAT", 5),
					luabind::value("STYLE_STATUS", 6)
				]
			];
		}

		void BindUser(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<User, Flags>("User")
				.def("getCID", &User::getCID, luabind::dependency(luabind::result, _1))
				.def("isFavorite", &User::isFavorite)
				.def("inc", &User::inc)
				.def("dec", &User::dec)
				.def("unique", &User::unique)
				.enum_("Flags") [
					luabind::value("ONLINE", (int)User::ONLINE),
					luabind::value("DCPLUSPLUS", (int)User::DCPLUSPLUS),
					luabind::value("PASSIVE", (int)User::PASSIVE),
					luabind::value("NMDC", (int)User::NMDC),
					luabind::value("BOT", (int)User::BOT),
					luabind::value("TLS", (int)User::TLS),
					luabind::value("OLD_CLIENT", (int)User::OLD_CLIENT),
					luabind::value("NO_ADC_1_0_PROTOCOL", (int)User::NO_ADC_1_0_PROTOCOL),
					luabind::value("NO_ADC_0_10_PROTOCOL", (int)User::NO_ADC_0_10_PROTOCOL),
					luabind::value("NO_ADCS_0_10_PROTOCOL", (int)User::NO_ADCS_0_10_PROTOCOL),
					luabind::value("PROTECTED", (int)User::PROTECTED),
					luabind::value("IGNORED", (int)User::IGNORED)
				]
			];
		}

		void BindOnlineUser(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<OnlineUser>("OnlineUser")
				.def("getIdentity", (Identity& (OnlineUser::*)())&OnlineUser::getIdentity, luabind::dependency(luabind::result, _1))
				.def("getUser", &wrappers::getUser<OnlineUser>)
				.def("getClient", (Client& (OnlineUser::*)())&OnlineUser::getClient, luabind::dependency(luabind::result, _1))
				.def("connect", &wrappers::OnlineUserConnect)
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
				.def("getUser", &wrappers::getUser<Identity>)
				.def("getSIDString", &Identity::getSIDString)
				.def("getStatus", &Identity::getStatus)
				.def("getTag", &Identity::getTag)
				.def("getParams", &wrappers::getIdentityParams)
				.enum_("ClientType") [
					luabind::value("CT_BOT", Identity::CT_BOT),
					luabind::value("CT_REGGED", Identity::CT_REGGED),
					luabind::value("CT_OP", Identity::CT_OP),
					luabind::value("CT_SU", Identity::CT_SU),
					luabind::value("CT_OWNER", Identity::CT_OWNER),
					luabind::value("CT_HUB", Identity::CT_HUB)
				]
				.enum_("StatusFlags") [
					luabind::value("NORMAL", Identity::NORMAL),
					luabind::value("AWAY", Identity::AWAY),
					luabind::value("SERVER", Identity::SERVER),
					luabind::value("FIREBALL", Identity::FIREBALL),
					luabind::value("TLS", Identity::TLS)
				]
			];
		}

		void BindCID(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<CID>("CID")
				.def("toBase32", (string (CID::*)() const)&CID::toBase32)
				.def("toHash", &CID::toHash)
				.def("isZero", &CID::isZero)
			];
		}

		void BindAdcCommand(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<AdcCommand>("AdcCommand")
				//.def(luabind::constructor<uint32_t, char>())
				//.def(luabind::constructor<uint32_t, const uint32_t, char>())
				//.def(luabind::constructor<const string&, bool>())
				//.def("parse", &AdcCommand::parse)
				//.def("addParam", (AdcCommand& (AdcCommand::*)(const string&, const string&))&AdcCommand::addParam, luabind::dependency(luabind::result, _1))
				//.def("getParam", (const string& (AdcCommand::*)(size_t) const)&AdcCommand::getParam)
				//.def("toString", (string (AdcCommand::*)(uint32_t, bool) const)&AdcCommand::toString)
				.scope [
					luabind::def("toSID", &AdcCommand::toSID),
					luabind::def("fromSID", &AdcCommand::fromSID)
				]
			];
		}

		void BindUserConnection(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<UserConnection, Flags>("UserConnection")
				.def("getRemoteIp", &UserConnection::getRemoteIp)
				.def("getPort", &UserConnection::getPort)
				.def("getHubUrl", &UserConnection::getHubUrl)
				.def("getUser", &wrappers::getUser<UserConnection>/*, luabind::dependency(luabind::result, _1)*/)
				.def("isSecure", &UserConnection::isSecure)
				.def("isTrusted", &UserConnection::isTrusted)
				.def("getCipherName", &UserConnection::getCipherName)
				.def("send", &UserConnection::sendRaw)
				.def("updated", &UserConnection::updated)
				.def("disconnect", &UserConnection::disconnect)
				.def("get", &UserConnection::get)
				.def("snd", &UserConnection::snd)
				.enum_("Flags") [
					luabind::value("NMDC", UserConnection::FLAG_NMDC),
					luabind::value("OP", UserConnection::FLAG_OP),
					luabind::value("UPLOAD", UserConnection::FLAG_UPLOAD),
					luabind::value("DOWNLOAD", UserConnection::FLAG_DOWNLOAD),
					luabind::value("STEALTH", UserConnection::FLAG_STEALTH),
					luabind::value("SECURE", UserConnection::FLAG_SECURE)
				]
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
				//.def(luabind::constructor<>())
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
				luabind::class_<Util>("Utils")
				.enum_("Paths") [
					luabind::value("PATH_GLOBAL_CONFIG", (int)Util::PATH_GLOBAL_CONFIG),
					luabind::value("PATH_USER_CONFIG", (int)Util::PATH_USER_CONFIG),
					luabind::value("PATH_USER_LOCAL", (int)Util::PATH_USER_LOCAL),
					luabind::value("PATH_RESOURCES", (int)Util::PATH_RESOURCES),
					luabind::value("PATH_DOWNLOADS", (int)Util::PATH_DOWNLOADS),
					luabind::value("PATH_FILE_LISTS", (int)Util::PATH_FILE_LISTS),
					luabind::value("PATH_HUB_LISTS", (int)Util::PATH_HUB_LISTS),
					luabind::value("PATH_NOTEPAD", (int)Util::PATH_NOTEPAD),
					luabind::value("PATH_EMOPACKS", (int)Util::PATH_EMOPACKS)
				]
				.scope [
					luabind::def("encodeURI", &Util::encodeURI),
					luabind::def("formatParams", &wrappers::makeParams),
					luabind::def("getVersion", &wrappers::getVersion),
					luabind::def("getIpCountry", &Util::getIpCountry),
					luabind::def("getLocalIp", &Util::getLocalIp),
					luabind::def("isPrivateIp", &Util::isPrivateIp),
					luabind::def("formatBytes", (std::string (*)(const string&))&Util::formatBytes),
					luabind::def("formatBytes", (std::string (*)(int64_t))&Util::formatBytes),
//					luabind::def("formatMessage", &Util::formatMessage),
					luabind::def("getStartTime", &Util::getStartTime),
					luabind::def("rand", (uint32_t (*)(uint32_t, uint32_t))&Util::rand),
					luabind::def("getPath", &Util::getPath)
				],

				luabind::def("LogMessage", &wrappers::LogMessage),
				luabind::namespace_("Text") [
					luabind::def("acpToUtf8", (string (*)(const string&))&Text::acpToUtf8),
					luabind::def("utf8ToAcp", (string (*)(const string&))&Text::utf8ToAcp),
					luabind::def("isAscii", (bool (*)(const char*))&Text::isAscii),
					luabind::def("getSystemCharset", &wrappers::getSystemCharset),
					luabind::def("convert", (string (*)(const string&, const string&, const string&))&Text::convert)
				]
			];
		}

		void BindShareManager(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<ShareManager>("ShareManager")
				.def("setHiddenDirectory", &ShareManager::setHiddenDirectory)
				.def("addDirectory", &ShareManager::addDirectory)
				.def("removeDirectory", &ShareManager::removeDirectory)
				.def("addExcludeFolder", &ShareManager::addExcludeFolder)
				.def("removeExcludeFolder", &ShareManager::removeExcludeFolder),

				luabind::def("getShareManager", &ShareManager::getInstance)
			];
		}

		void BindConnectionManager(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<ConnectionManager>("ConnectionManager")
				.def("addNmdcFeature", &ConnectionManager::addNmdcFeat)
				.def("addAdcFeature", &ConnectionManager::addAdcFeat),
				
				luabind::def("getConnectionManager", &ConnectionManager::getInstance)
			];
		}

		void BindQueueManager(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<QueueManager>("QueueManager")
				.def("add", &wrappers::QueueManager_addFile)
				.def("remove", &QueueManager::remove),

				luabind::def("getQueueManager", &QueueManager::getInstance)
			];
		}

		void BindTTHValue(lua_State* /*L*/) {

		}

		void BindSettingsManager(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<SettingsManager>("SettingsManager")
				.def("getString", &SettingsManager::getString)
				.def("getInt", &SettingsManager::getInt),

				luabind::class_<rsxppSettingsManager>("rsxppSettingsManager")
				.def("addAction", &rsxppSettingsManager::addAction)
				.def("getActionId", &rsxppSettingsManager::getActionId)
				.def("getExtSetting", &rsxppSettingsManager::getExtSetting)
				.def("setExtSetting", &rsxppSettingsManager::setExtSetting),

				luabind::def("getSettingsManager", &SettingsManager::getInstance),
				luabind::def("getRSXSettingsManager", &rsxppSettingsManager::getInstance)
			];
		}

		void BindClientManager(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<ClientManager>("ClientManager")
				.def("openClient", &ClientManager::openHub)
				.def("closeClient", &ClientManager::closeHub)
				.def("getHubs", &wrappers::getHubs)
				.def("getUsers", &wrappers::getOnlineUsers),

				luabind::def("getClientManager", &ClientManager::getInstance)
			];
		}

		void BindFavoriteManager(lua_State* L) {
			luabind::module(L, "dcpp") [
				luabind::class_<FavoriteManager>("FavoriteManager")
				.def("addUserCommand", &FavoriteManager::addUserCommand, luabind::discard_result),

				luabind::class_<UserCommand>("UserCommand")
				.def("getID", &UserCommand::getId)
				.def("getType", &UserCommand::getType)
				.def("getContext", &UserCommand::getCtx)
				.def("getName", &UserCommand::getName)
				.def("getCommand", &UserCommand::getCommand)
				.def("getHub", &UserCommand::getHub)
				.enum_("UCFlags") [
					luabind::value("FLAG_NOSAVE", UserCommand::FLAG_NOSAVE),
					luabind::value("FLAG_LUAMENU", UserCommand::FLAG_LUAMENU)
				]
				.enum_("UCType") [
					luabind::value("TYPE_SEPARATOR", (int)UserCommand::TYPE_SEPARATOR),
					luabind::value("TYPE_RAW", (int)UserCommand::TYPE_RAW),
					luabind::value("TYPE_RAW_ONCE", (int)UserCommand::TYPE_RAW_ONCE),
					luabind::value("TYPE_REMOVE", (int)UserCommand::TYPE_REMOVE),
					luabind::value("TYPE_CLEAR", (int)UserCommand::TYPE_CLEAR)
				]
				.enum_("UCContext") [
					luabind::value("CONTEXT_HUB", (int)UserCommand::CONTEXT_HUB),
					luabind::value("CONTEXT_CHAT", (int)UserCommand::CONTEXT_CHAT),
					luabind::value("CONTEXT_SEARCH", (int)UserCommand::CONTEXT_SEARCH),
					luabind::value("CONTEXT_FILELIST", (int)UserCommand::CONTEXT_FILELIST),
					luabind::value("CONTEXT_MASK", (int)UserCommand::CONTEXT_MASK)
				],

				luabind::def("getFavoriteManager", &FavoriteManager::getInstance)
			];
		}
	}
}
