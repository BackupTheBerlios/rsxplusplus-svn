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

#ifndef DCPP_INTERFACE_CORE_HPP
#define DCPP_INTERFACE_CORE_HPP

#define MAKE_VER(n1, n2, n3, n4)	(((uint64_t)n1 & 0xFFFF) << 48 | ((uint64_t)n2 & 0xFFFF) << 32 | ((uint64_t)n3 & 0xFFFF) << 16 | ((uint64_t)n4 & 0xFFFF))
#define VER_MAJ(v)					(uint32_t)(((uint64_t)v & 0xFFFF000000000000) >> 48)
#define VER_MIN(v)					(uint32_t)(((uint64_t)v & 0x0000FFFF00000000) >> 32)
#define VER_REV(v)					(uint32_t)(((uint64_t)v & 0x00000000FFFF0000) >> 16)
#define VER_BLD(v)					(uint32_t)(((uint64_t)v & 0x000000000000FFFF))

// PluginSDK version (include in PluginInformation)
#define SDK_VERSION MAKE_VER(3, 0, 5, 0)

#ifndef _WIN32
#include <stdint.h>

// some non-windows stuff... todo 4sure...
typedef unsigned short wchar_t;
#else

#if (!defined(_STLPORT_VERSION)) || (_STLPORT_VERSION < 0x600)
	typedef signed __int8 int8_t;
	typedef signed __int16 int16_t;
	typedef signed __int32 int32_t;
	typedef signed __int64 int64_t;

	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;
#endif

#endif // _WIN32

namespace dcpp {
	namespace interfaces {
		class string;
		class stringList;
		class stringMap;
		class AdcCommand;
		class UserConnection;

		class Memory {
		public:
			virtual char* alloc(size_t size) = 0;
			virtual void free(char* data) = 0;

			virtual dcpp::interfaces::string* getString(const char* buf = 0) = 0;
			virtual void putString(dcpp::interfaces::string* str) = 0;

			virtual dcpp::interfaces::stringList* getStringList(size_t size = 0) = 0;
			virtual void putStringList(dcpp::interfaces::stringList* list) = 0;

			virtual dcpp::interfaces::stringMap* getStringMap() = 0;
			virtual void putStringMap(dcpp::interfaces::stringMap* map) = 0;

			virtual dcpp::interfaces::AdcCommand* getAdcCommand(uint32_t command) = 0;
			virtual dcpp::interfaces::AdcCommand* getAdcCommand(uint32_t command, const uint32_t target, char type) = 0;
			virtual dcpp::interfaces::AdcCommand* getAdcCommand(const char* str, bool nmdc) = 0;
			virtual void putAdcCommand(dcpp::interfaces::AdcCommand* command) = 0;
		};

		class Utils {
		public:
			virtual dcpp::interfaces::string* formatParams(const char* format, dcpp::interfaces::stringMap* params) = 0;
			virtual dcpp::interfaces::string* convertFromWideToUtf8(const wchar_t* str) = 0;
			virtual dcpp::interfaces::string* convertFromWideToAcp(const wchar_t* str) = 0;
			virtual dcpp::interfaces::string* convertFromAcpToUtf8(const char* str) = 0;
			virtual dcpp::interfaces::string* convertFromUtf8ToAcp(const char* str) = 0;

			virtual uint32_t toSID(const char* sid) = 0;
			virtual dcpp::interfaces::string* fromSID(uint32_t sid) = 0;
			virtual uint32_t toFourCC(const char* cc) = 0;
			virtual dcpp::interfaces::string* fromFourCC(uint32_t cc) = 0;

			virtual const char* getPath(int type) const = 0;
		};

		namespace Paths {
			static const int PATH_GLOBAL_CONFIG = 0;
			static const int PATH_USER_CONFIG = 1;
			static const int PATH_USER_LOCAL = 2;
			static const int PATH_RESOURCES = 3;
			static const int PATH_LOCALE = 4;
			static const int PATH_DOWNLOADS = 5;
			static const int PATH_FILE_LISTS = 6;
			static const int PATH_HUB_LISTS = 7;
			static const int PATH_NOTEPAD = 8;
			static const int PATH_EMOPACKS = 9;
		};

		class OnlineUser;
		class Hub;

		class HubManagerListener {
		public:
			virtual void onHubManager_HubConnected(dcpp::interfaces::Hub*) throw() { }
			virtual void onHubManager_HubUpdated(dcpp::interfaces::Hub*) throw() { }
			virtual void onHubManager_HubDisconnected(dcpp::interfaces::Hub*) throw() { }

			// todo
			virtual void onHubManager_UserUpdated(dcpp::interfaces::OnlineUser*) throw() { }
		};

		class TimerListener {
		public:
			virtual void onTimer_Second(uint64_t) throw() { }
			virtual void onTimer_Minute(uint64_t) throw() { }
		};

		class CoreListener {
		public:
			virtual void onCore_LoadComplete() throw() { }
			virtual void onCore_UnloadPrepare() throw() { }
			virtual void onCore_SettingsLoad() throw() { }
			virtual void onCore_SettingsSave() throw() { }
		};

		class ConnectionManagerListener {
		public:
			virtual void onConnectionManager_ConnectionCreated(dcpp::interfaces::UserConnection*) throw() { }
			virtual void onConnectionManager_ConnectionDestroyed(dcpp::interfaces::UserConnection*) throw() { }
		};

		class Core {
		public:
			virtual void log(const char* msg) = 0;
	
			virtual void addEventListener(dcpp::interfaces::HubManagerListener* listener) = 0;
			virtual void addEventListener(dcpp::interfaces::ConnectionManagerListener* listener) = 0;
			virtual void addEventListener(dcpp::interfaces::TimerListener* listener) = 0;
			virtual void addEventListener(dcpp::interfaces::CoreListener* listener) = 0;

			virtual void remEventListener(dcpp::interfaces::HubManagerListener* listener) = 0;
			virtual void remEventListener(dcpp::interfaces::ConnectionManagerListener* listener) = 0;
			virtual void remEventListener(dcpp::interfaces::TimerListener* listener) = 0;
			virtual void remEventListener(dcpp::interfaces::CoreListener* listener) = 0;

			virtual dcpp::interfaces::Memory* getMemoryManager() = 0;
			virtual dcpp::interfaces::Utils* getUtils() = 0;

			virtual const char* getPluginSetting(const char* key, const char* defaultValue = 0) = 0;
			virtual void setPluginSetting(const char* key, const char* value) = 0;

			virtual bool getCoreSetting(const char* key, const char*& value) = 0;
			virtual bool getCoreSetting(const char* key, int& value) = 0;
			virtual bool getCoreSetting(const char* key, int64_t& value) = 0;

			virtual bool setCoreSetting(const char* key, const char* value) = 0;
			virtual bool setCoreSetting(const char* key, const int& value) = 0;
			virtual bool setCoreSetting(const char* key, const int64_t& value) = 0;
		};
	} // namespace interfaces
} // namespace dcpp

#endif
