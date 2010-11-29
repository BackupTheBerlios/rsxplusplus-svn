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

#ifndef DCPP_INTERFACE_HUB_HPP
#define DCPP_INTERFACE_HUB_HPP

namespace dcpp {
	namespace interfaces {
		class string;
		class Identity;
		class OnlineUser;
		class HubListener;
		class ChatMessage;

		class Hub {
		public:
			virtual void sendMessage(const char* msg, bool thirdPerson = false) = 0;
			virtual void sendPrivateMessage(dcpp::interfaces::OnlineUser* u, const char* msg, bool thirdPerson) = 0;
			virtual void sendData(const void* data, size_t len) = 0;

			//virtual void dcpp::interfaces::OnlineUser* findUser(const char* nick) = 0;

			virtual void disconnect(bool graceless) = 0;
			virtual bool isConnected() const = 0;
			virtual bool isSecure() const = 0;
			virtual bool isTrusted() const = 0;

			virtual void reconnect() = 0;
			virtual void shutdown() = 0;
			virtual bool isActive() const = 0;

			virtual const char* getHubAddress() = 0;
			virtual const char* getHubURL() = 0;
			virtual const char* getHubIP() = 0;
			//virtual const char* getLocalIP() = 0;
			virtual uint16_t getPort() const = 0;

			virtual bool isAccountRegistered() const = 0;
			virtual const char* getAccountNick() = 0;
			virtual const char* getAccountPassword() = 0;

			virtual dcpp::interfaces::Identity* getAccountIdentity() = 0;
			virtual dcpp::interfaces::Identity* getIdentity() = 0;

			virtual void mutex(bool lock) = 0;
			virtual void addEventListener(dcpp::interfaces::HubListener* listener) = 0;
			virtual void remEventListener(dcpp::interfaces::HubListener* listener) = 0;
		};

		class HubListener {
		public:
			// notifications, can't stop them
			virtual void onHub_Connecting(dcpp::interfaces::Hub*) throw() 							{ }
			virtual void onHub_Connected(dcpp::interfaces::Hub*) throw() 								{ }
			virtual void onHub_UserUpdated(dcpp::interfaces::Hub*, interfaces::OnlineUser*) throw() 	{ }
			virtual void onHub_UserRemoved(dcpp::interfaces::Hub*, interfaces::OnlineUser*) throw() 	{ }
			virtual void onHub_Redirect(dcpp::interfaces::Hub*, const char*) throw() 					{ }
			virtual void onHub_Failed(dcpp::interfaces::Hub*, const char*) throw() 					{ }
			virtual void onHub_HubUpdated(dcpp::interfaces::Hub*) throw() 							{ }
			virtual void onHub_HubFull(dcpp::interfaces::Hub*) throw() 								{ }
			virtual void onHub_AccountNickTaken(dcpp::interfaces::Hub*) throw() 						{ }

			//events, we can stop them by changing last param to true
			virtual void onHub_IncomingCommand(dcpp::interfaces::Hub*, const char*, bool&) throw() { }
			virtual void onHub_OutgoingCommand(dcpp::interfaces::Hub*, const char*, bool&) throw() { }
			virtual void onHub_IncomingMessage(dcpp::interfaces::Hub*, dcpp::interfaces::ChatMessage*, bool&) throw() { }
			virtual void onHub_OutgoingMessage(dcpp::interfaces::Hub*, dcpp::interfaces::ChatMessage*, bool&) throw() { }
		};

	} // namespace interfaces
} // namespace dcpp

#endif

