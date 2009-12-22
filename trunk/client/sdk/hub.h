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

#ifndef DCPP_SDK_HUB
#define DCPP_SDK_HUB

#define DCPP_EVENT_HUB "Hub/"
#define DCPP_CALL_HUB "Hub/"

#define DCPP_EVENT_HUB_CREATED				1 //"Hub/Created"
#define DCPP_EVENT_HUB_DESTROYED			2 //"Hub/Destroyed"
#define DCPP_EVENT_HUB_CONNECTING			3 //"Hub/Connecting"
#define DCPP_EVENT_HUB_CONNECTED			4 //"Hub/Connected"
#define DCPP_EVENT_HUB_DISCONNECTED			5 //"Hub/Disconnected"
#define DCPP_EVENT_HUB_REDIRECTED			6 //"Hub/Redirected"
#define DCPP_EVENT_HUB_CHAT_MESSAGE_IN		7 //"Hub/ChatMessageIn"
#define DCPP_EVENT_HUB_CHAT_MESSAGE_OUT		8 //"Hub/ChatMessageOut"

#define DCPP_CALL_HUB_OPEN					"Hub/Open"
#define DCPP_CALL_HUB_CLOSE					"Hub/Close"
#define DCPP_CALL_HUB_REDIRECT				"Hub/Redirect"

#define DCPP_HUB_FIELD_NICK					"NI"
#define DCPP_HUB_FIELD_DESCRIPTION			"DE"
#define DCPP_HUB_FIELD_EMAIL				"EM"
#define DCPP_HUB_FIELD_CONNECTION			"US"

#define DCPP_CALL_HUB_GET_HUB_FIELD(x)		"Hub/Identity/Hub/Get/" x
#define DCPP_CALL_HUB_GET_MY_FIELD(x)		"Hub/Identity/My/Get/" x
#define DCPP_CALL_HUB_SET_HUB_FIELD(x)		"Hub/Identity/Hub/Set/" x
#define DCPP_CALL_HUB_SET_MY_FIELD(x)		"Hub/Identity/My/Set/" x

#define DCPP_CALL_HUB_SEND_MESSAGE			"Hub/SendMessage"
#define DCPP_CALL_HUB_SEND_USER_COMMAND		"Hub/SendUserCommand"
#define DCPP_CALL_HUB_SOCKET_WRITE			"Hub/SocketWrite"
#define DCPP_CALL_HUB_CHAT_WINDOW_WRITE		"Hub/ChatWindowWrite"

typedef struct {
	const char* address;
	const char* ip;
	uint16_t port;
	uint8_t isAdc;
	uint8_t isSecured;
} dcppHubInformation;

typedef struct {
	const char* message;
	uint8_t thirdPerson;

	dcpp_ptr_t from;
	dcpp_ptr_t to;
	dcpp_ptr_t replyTo;
} dcppChatMessage;

#endif

/**
 * @file
 * $Id$
 */
