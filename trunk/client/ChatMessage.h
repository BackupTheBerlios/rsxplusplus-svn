/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CHAT_MESSAGE_H
#define DCPLUSPLUS_DCPP_CHAT_MESSAGE_H

#include "forward.h"
#include "sdk/interfaces/ChatMessage.hpp"

namespace dcpp {

struct ChatMessage : public interfaces::ChatMessage {
	//RSX++
	ChatMessage() : thirdPerson(false), timestamp(0) { }
	ChatMessage(const string& _text) : text(_text) { }
	ChatMessage(const string& _text, const OnlineUserPtr& _from) : text(_text), from(_from), timestamp(0), thirdPerson(false) { }
	ChatMessage(const string& _text, const OnlineUserPtr& _from, const OnlineUserPtr& _to, const OnlineUserPtr& _replyTo) : 
		text(_text), from(_from), to(_to), replyTo(_replyTo), timestamp(0), thirdPerson(false) { }
	//END
	string text;

	OnlineUserPtr from;
	OnlineUserPtr to;
	OnlineUserPtr replyTo;

	bool thirdPerson;
	time_t timestamp;

	string format() const;

	//RSX++ interface impl
	const char* getText() { return text.c_str(); }
	void setText(const char* msg) { text = msg; }

	interfaces::OnlineUser* getFrom();
	interfaces::OnlineUser* getTo();
	interfaces::OnlineUser* getReplyTo();

	bool getThirdPerson() { return thirdPerson; }
	void setThirdPerson(bool t) { thirdPerson = t; }

	time_t getTimeStamp() { return timestamp; }
	void setTimeStamp(time_t ts) { timestamp = ts; }
	//END
};

} // namespace dcpp

#endif // !defined(DCPLUSPLUS_DCPP_CHAT_MESSAGE_H)
