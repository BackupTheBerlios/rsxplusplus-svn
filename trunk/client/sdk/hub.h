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

#define DCPP_EVENT_HUB	"Hub/"
#define DCPP_CALL_HUB	"Hub/"

/*************************************************************************************/
/* events section */

/* hub object is created, but no connection is made
*/
#define DCPP_EVENT_HUB_CREATED				1

/* hub object is destroyed, connection was already closed
*/
#define DCPP_EVENT_HUB_DESTROYED			2

/* hub attempts to connect at given address
*/
#define DCPP_EVENT_HUB_CONNECTING			3

/* connection at given address is made
*/
#define DCPP_EVENT_HUB_CONNECTED			4

/* connection is closed, hub object still exists in memory
*/
#define DCPP_EVENT_HUB_DISCONNECTED			5

/* hub was redirected to new address
*/
#define DCPP_EVENT_HUB_REDIRECTED			6

/* incoming chat message. source can be main chat or private chat
*/
#define DCPP_EVENT_HUB_CHAT_MESSAGE			7

/* our attempt to send a message to the hub
*/
#define DCPP_EVENT_HUB_CHAT_SEND_LINE		8

/* raw line read from socket (incoming/outgoing)
*/
#define DCPP_EVENT_HUB_LINE					9

/*************************************************************************************/
/* calls section */

/* create new hub object and eventually open gui frame for it
*/
#define DCPP_CALL_HUB_OPEN					"Hub/Open"

/* close existing hub
*/
#define DCPP_CALL_HUB_CLOSE					"Hub/Close"

/* redirect existing hub to given address (current connection will be closed)
*/
#define DCPP_CALL_HUB_REDIRECT				"Hub/Redirect"

/* send regular chat message to the hub
   param1 (dcpp_param):			hubPtr - hub object
   param2 (const char*):		message
   param3 (bool):				thirdPerson - send as /me if not 0
*/
#define DCPP_CALL_HUB_SEND_CHAT_MESSAGE		"Hub/SendChatMessage"

/* DEPRECATED */
#define DCPP_CALL_HUB_SEND_USER_COMMAND		"Hub/SendUserCommand"

/* write n bytes from line to the hub
   param1 (dcpp_param):			hubPtr - hub object
   param2 (const char*):		line
   param3 (size_t):				n
*/
#define DCPP_CALL_HUB_LINE_WRITE			"Hub/LineWrite"

/* add message to hub gui frame (but don't send it to the hub)
   param1 (dcpp_param):		hubPtr - hub object
   param2 (const char*):	message
   param3 (int):			format - chat format used for this message
*/
#define DCPP_CALL_HUB_CHAT_WINDOW_WRITE		"Hub/ChatWindowWrite"

/* format dcppChatMessage to regular string
   param1 (dcppChatMessage*):	hubPtr - hub object
   param2 (dcppBuffer*):		buffer - pointer to dcppBuffer structure with allocated memory to copy string
*/
#define DCPP_CALL_HUB_FORMAT_CHAT_MESSAGE	"Hub/FormatChatMessage"

/* dispatch line as it would come from hub (e.g to emulate hub command)
   param1 (dcpp_param):			hubPtr - hub object
   param2 (const char*):		line
*/
#define DCPP_CALL_HUB_DISPATCH_LINE			"Hub/DispatchLine"

/* get basic information about hub (see dcppHubInfo structure)
   param1 (dcpp_param):			hubPtr - hub object
   param2 (dcppHubInfo*):		info - pointer to structure dcppHubInfo
*/
#define DCPP_CALL_HUB_GET_HUB_INFO			"Hub/GetHubInfo"

/* calls to interact with identity values
   available calls are for hub identity and "my" identity
   which represent us as user connected to the hub
*/
/* getters */
#define DCPP_CALL_HUB_GET_HUB_FIELD(x)		"Hub/Identity/Hub/Get/" x
#define DCPP_CALL_HUB_GET_MY_FIELD(x)		"Hub/Identity/My/Get/" x
/* setters */
#define DCPP_CALL_HUB_SET_HUB_FIELD(x)		"Hub/Identity/Hub/Set/" x
#define DCPP_CALL_HUB_SET_MY_FIELD(x)		"Hub/Identity/My/Set/" x

/* identity fields (use to above calls) */
#define DCPP_HUB_FIELD_NICK					"NI"
#define DCPP_HUB_FIELD_DESCRIPTION			"DE"
#define DCPP_HUB_FIELD_EMAIL				"EM"
#define DCPP_HUB_FIELD_CONNECTION			"US"
#define DCPP_HUB_FIELD_VERSION				"VE"
#define DCPP_HUB_FIELD_IP_V4				"I4"
#define DCPP_HUB_FIELD_UDP_V4				"U4"

/*************************************************************************************/
/* data structures sections - most of them are self-explained */

typedef struct {
	uint16_t		port;
	uint8_t			isAdc;
	uint8_t			isSecured;
	const char*		address;
	const char*		url;
	const char*		ip;
} dcppHubInfo;

typedef struct {
	dcpp_param		hubPtr;

	dcpp_param		from;
	dcpp_param		to;
	dcpp_param		replyTo;
	uint64_t		timestamp;
	const char*		message;
	uint8_t			thirdPerson;
	int8_t			incoming;
} dcppChatMessage;

typedef struct {
	dcpp_param		hubPtr;

	dcpp_param		length;
	const char*		line;
	int8_t			incoming;
} dcppHubLine;

#endif

/**
 * @file
 * $Id$
 */
