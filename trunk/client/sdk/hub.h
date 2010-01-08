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

#ifndef DCPP_SDK_HUB
#define DCPP_SDK_HUB

#define DCPP_EVENT_HUB "Hub/"
#define DCPP_CALL_HUB "Hub/"

#define DCPP_EVENT_HUB_CREATED				1
#define DCPP_EVENT_HUB_DESTROYED			2
#define DCPP_EVENT_HUB_CONNECTING			3
#define DCPP_EVENT_HUB_CONNECTED			4
#define DCPP_EVENT_HUB_DISCONNECTED			5
#define DCPP_EVENT_HUB_REDIRECTED			6
//this might be a main chat or private chat message
#define DCPP_EVENT_HUB_CHAT_MESSAGE			7
//this is outgoing chat message
#define DCPP_EVNET_HUB_CHAT_SEND_LINE		8
//raw protocol lines read/write from/to socket
#define DCPP_EVENT_HUB_LINE					9

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

//params: dcpp_ptr_t hubPtr, const char* msg, bool thirdPerson
#define DCPP_CALL_HUB_SEND_CHAT_MESSAGE		"Hub/SendChatMessage"
//params: dcpp_ptr_t hubPtr, const char* cmd
#define DCPP_CALL_HUB_SEND_USER_COMMAND		"Hub/SendUserCommand"
//params: dcpp_ptr_t hubPtr, const char* line, size_t len
#define DCPP_CALL_HUB_LINE_WRITE			"Hub/LineWrite"
//params: dcpp_ptr_t hubPtr, const char* msg, int format
#define DCPP_CALL_HUB_CHAT_WINDOW_WRITE		"Hub/ChatWindowWrite"
//params: dcppChatMessage*, dcppBuffer*
#define DCPP_CALL_HUB_FORMAT_CHAT_MESSAGE	"Hub/FormatChatMessage"
//params: dcpp_ptr_t hubPtr, const char* line
#define DCPP_CALL_HUB_DISPATCH_LINE			"Hub/DispatchLine"
/*
typedef struct {
	uint16_t port;
	uint8_t isAdc;
	uint8_t isSecured;
	const char* address;
	const char* ip;
} dcppHubInfo;
*/
typedef struct {
	dcpp_ptr_t hubPtr;

	dcpp_ptr_t from;
	dcpp_ptr_t to;
	dcpp_ptr_t replyTo;
	uint64_t timestamp;
	const char* message;
	uint8_t thirdPerson;
} dcppChatMessage;

typedef struct {
	dcpp_ptr_t hubPtr;

	dcpp_ptr_t length;
	const char* line;
} dcppHubLine;

#endif

/**
 * @file
 * $Id$
 */
