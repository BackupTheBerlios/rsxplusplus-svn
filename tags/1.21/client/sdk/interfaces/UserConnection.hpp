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

#ifndef DCPP_INTERFACE_USER_CONNECTION_HPP
#define DCPP_INTERFACE_USER_CONNECTION_HPP

namespace dcpp {
	namespace interfaces {
		class string;
		class stringList;
		class UserConnectionListener;
		class AdcCommand;

		namespace UserConnectionFlags {
			static const int FLAG_NMDC = 0x01;
			static const int FLAG_OP = 0x02;
			static const int FLAG_UPLOAD = 0x04;
			static const int FLAG_DOWNLOAD = 0x08;
			static const int FLAG_INCOMING = 0x10;
			static const int FLAG_ASSOCIATED = 0x20;
			static const int FLAG_SUPPORTS_MINISLOTS = 0x40;
			static const int FLAG_SUPPORTS_XML_BZLIST = 0x80;
			static const int FLAG_SUPPORTS_ADCGET = 0x100;
			static const int FLAG_SUPPORTS_ZLIB_GET = 0x200;
			static const int FLAG_SUPPORTS_TTHL = 0x400;
			static const int FLAG_SUPPORTS_TTHF = 0x800;
			static const int FLAG_STEALTH = 0x1000;
			static const int FLAG_SECURE = 0x2000;
		}

		class UserConnection {
		public:
			virtual void disconnect(bool graceless = false) = 0;
			virtual bool isSecure() const = 0;
			virtual bool isTrusted() const = 0;
			virtual uint16_t getPort() const = 0;
			virtual const char* getIp() const = 0;
			virtual const char* getHubURL() const = 0;
			virtual void updated() = 0;

			virtual void setFlags(int flags) = 0;
			virtual bool isFlagSet(int flag) = 0;
			virtual bool isAnyFlagSet(int flags) = 0;

			virtual void sendData(const void* data, size_t len) = 0;
			virtual void sendLine(const char* line) = 0;
			virtual void parseLine(const char* line) = 0;

			virtual void addEventListener(dcpp::interfaces::UserConnectionListener* listener) = 0;
			virtual void remEventListener(dcpp::interfaces::UserConnectionListener* listener) = 0;
		};

		class UserConnectionListener {
		public:
			virtual void onUserConnection_IncomingLine(dcpp::interfaces::UserConnection*, const char*, bool&) throw() { }
			virtual void onUserConnection_OutgoingLine(dcpp::interfaces::UserConnection*, const char*, bool&) throw() { }

			virtual void onUserConnection_BytesSent(dcpp::interfaces::UserConnection*, size_t, size_t) throw() { }
			virtual void onUserConnection_Connected(dcpp::interfaces::UserConnection*) throw() { }
			virtual void onUserConnection_Data(dcpp::interfaces::UserConnection*, const uint8_t*, size_t) throw() { }
			virtual void onUserConnection_Failed(dcpp::interfaces::UserConnection*, const char*) throw() { }
			virtual void onUserConnection_CLock(dcpp::interfaces::UserConnection*, const char*, const char*) throw() { }
			virtual void onUserConnection_Key(dcpp::interfaces::UserConnection*, const char*) throw() { }
			virtual void onUserConnection_Direction(dcpp::interfaces::UserConnection*, const char*, const char*) throw() { }
			virtual void onUserConnection_Get(dcpp::interfaces::UserConnection*, const char*, int64_t) throw() { }
			virtual void onUserConnection_Send(dcpp::interfaces::UserConnection*) throw() { }
			virtual void onUserConnection_MaxedOut(dcpp::interfaces::UserConnection*, const char* = 0) throw() { }
			virtual void onUserConnection_ModeChange(dcpp::interfaces::UserConnection*) throw() { }
			virtual void onUserConnection_MyNick(dcpp::interfaces::UserConnection*, const char*) throw() { }
			virtual void onUserConnection_TransmitDone(dcpp::interfaces::UserConnection*) throw() { }
			virtual void onUserConnection_Supports(dcpp::interfaces::UserConnection*, dcpp::interfaces::stringList*) throw() { }
			virtual void onUserConnection_FileNotAvailable(dcpp::interfaces::UserConnection*) throw() { }
			virtual void onUserConnection_Updated(dcpp::interfaces::UserConnection*) throw() { }
			
			virtual void onUserConnection_AdcCommand(dcpp::interfaces::UserConnection*, dcpp::interfaces::AdcCommand*) throw() { }
		};

	} // namespace interfaces
} // namespace dcpp

#endif
