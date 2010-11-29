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

#ifndef DCPP_INTERFACE_CHAT_MESSAGE_HPP
#define DCPP_INTERFACE_CHAT_MESSAGE_HPP

#include "RefInterface.hpp"

namespace dcpp {
	namespace interfaces {
		class OnlineUser;

		class ChatMessage {
		public:
			virtual const char* getText() = 0;
			virtual void setText(const char* msg) = 0;
			virtual dcpp::interfaces::OnlineUser* getFrom() = 0;
			virtual dcpp::interfaces::OnlineUser* getTo() = 0;
			virtual dcpp::interfaces::OnlineUser* getReplyTo() = 0;

			virtual bool getThirdPerson() = 0;
			virtual void setThirdPerson(bool t) = 0;

			virtual time_t getTimeStamp() = 0;
			virtual void setTimeStamp(time_t ts) = 0;
		};
	} // namespace interfaces
} // namespace dcpp

#endif
