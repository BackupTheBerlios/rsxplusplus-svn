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

#ifndef DCPP_PLUGIN_EVENTS
#define DCPP_PLUGIN_EVENTS

#define DCPP_DROP_EVENT						1
#define DCPP_PROCESS_EVENT					0

#define DCPP_SET_EVENT(x, y) x + y
#define DCPP_SET_CALL(x, y) x + y + 500 // 500 offset *should* be fine

/** Listeners:
  * DCPP_CORE
  * DCPP_HUB
  * DCPP_USER
  * DCPP_CONNECTION
  */

#define DCPP_CORE							1000
//Events
#define DCPP_CORE_INIT_OPEN					DCPP_SET_EVENT(DCPP_CORE, 1)
#define DCPP_CORE_INIT_CLOSE				DCPP_SET_EVENT(DCPP_CORE, 2)
#define DCPP_CORE_INIT_FINISHED				DCPP_SET_EVENT(DCPP_CORE, 3)
#define DCPP_CORE_LUA_INIT					DCPP_SET_EVENT(DCPP_CORE, 4)
#define DCPP_CORE_SETTINGS_UPDATED			DCPP_SET_EVENT(DCPP_CORE, 5)
#define DCPP_CORE_TIMER_SECOND				DCPP_SET_EVENT(DCPP_CORE, 6)
#define DCPP_CORE_TIMER_MINUTE				DCPP_SET_EVENT(DCPP_CORE, 7)
//Calls
#define DCPP_CORE_GET_SETTING				DCPP_SET_CALL(DCPP_CORE, 1)
#define DCPP_CORE_SET_SETTING				DCPP_SET_CALL(DCPP_CORE, 2)


#define DCPP_HUB							2000
//Events
#define DCPP_HUB_CREATED					DCPP_SET_EVENT(DCPP_HUB, 1)
#define DCPP_HUB_CONNECTING					DCPP_SET_EVENT(DCPP_HUB, 2)
#define DCPP_HUB_CONNECTED					DCPP_SET_EVENT(DCPP_HUB, 3)
#define DCPP_HUB_DISCONNECTED				DCPP_SET_EVENT(DCPP_HUB, 4)
#define DCPP_HUB_REDIRECTING				DCPP_SET_EVENT(DCPP_HUB, 5)
#define DCPP_HUB_DESTROYED					DCPP_SET_EVENT(DCPP_HUB, 6)
#define DCPP_HUB_MESSAGE_IN					DCPP_SET_EVENT(DCPP_HUB, 7)
#define DCPP_HUB_MESSAGE_OUT				DCPP_SET_EVENT(DCPP_HUB, 8)
#define DCPP_HUB_PRIV_MESSAGE_IN			DCPP_SET_EVENT(DCPP_HUB, 9)
#define DCPP_HUB_PRIV_MESSAGE_OUT			DCPP_SET_EVENT(DCPP_HUB, 10)
//Calls
#define DCPP_HUB_OPEN						DCPP_SET_CALL(DCPP_HUB, 1)
#define DCPP_HUB_CLOSE						DCPP_SET_CALL(DCPP_HUB, 2)
#define DCPP_HUB_REDIRECT					DCPP_SET_CALL(DCPP_HUB, 3)
#define DCPP_HUB_FIELD_GET					DCPP_SET_CALL(DCPP_HUB, 4)
#define DCPP_HUB_FIELD_SET					DCPP_SET_CALL(DCPP_HUB, 5)
#define DCPP_HUB_FIELD_MY_GET				DCPP_SET_CALL(DCPP_HUB, 6)
#define DCPP_HUB_FIELD_MY_SET				DCPP_SET_CALL(DCPP_HUB, 7)
#define DCPP_HUB_SOCKET_WRITE				DCPP_SET_CALL(DCPP_HUB, 8)
#define DCPP_HUB_SEND_MESSAGE				DCPP_SET_CALL(DCPP_HUB, 9)
#define DCPP_HUB_SEND_USER_COMMAND			DCPP_SET_CALL(DCPP_HUB, 10)
#define DCPP_HUB_WRITE_LINE					DCPP_SET_CALL(DCPP_HUB, 11)



#define DCPP_USER							3000
//Events
#define DCPP_USER_CONNECTED					DCPP_SET_EVENT(DCPP_USER, 1)
#define DCPP_USER_DISCONNECTED				DCPP_SET_EVENT(DCPP_USER, 2)
//Calls
#define DCPP_USER_FIELD_GET					DCPP_SET_CALL(DCPP_USER, 1)
#define DCPP_USER_FIELD_SET					DCPP_SET_CALL(DCPP_USER, 2)



#define DCPP_CONNECTION						4000
//Events
#define DCPP_CONNECTION_MESSAGE_IN			DCPP_SET_EVENT(DCPP_CONNECTION, 1)
#define DCPP_CONNECTION_MESSAGE_OUT			DCPP_SET_EVENT(DCPP_CONNECTION, 2)
//Calls
#define DCPP_CONNECTION_WRITE				DCPP_SET_CALL(DCPP_CONNECTION, 1)
#define DCPP_CONNECTION_DISCONNECT			DCPP_SET_CALL(DCPP_CONNECTION, 2)
#define DCPP_CONNECTION_FLAG_GET			DCPP_SET_CALL(DCPP_CONNECTION, 3)
#define DCPP_CONNECTION_FLAG_SET			DCPP_SET_CALL(DCPP_CONNECTION, 4)
#define DCPP_CONNECTION_FLAG_UNSET			DCPP_SET_CALL(DCPP_CONNECTION, 5)
#define DCPP_CONNECTION_FLAG_ISSET			DCPP_SET_CALL(DCPP_CONNECTION, 6)



#define DCPP_CONFIG							5000
//Events
#define DCPP_CONFIG_REFRESHED				DCPP_SET_EVENT(DCPP_CONFIG, 1)
//Calls
#define DCPP_CONFIG_GET						DCPP_SET_CALL(DCPP_CONFIG, 1)
#define DCPP_CONFIG_SET						DCPP_SET_CALL(DCPP_CONFIG, 2)



#define DCPP_UTILS							6000
//Calls
#define DCPP_UTILS_LOG_MESSAGE				DCPP_SET_CALL(DCPP_UTILS, 1)
#define DCPP_UTILS_FORMAT_PARAMS			DCPP_SET_CALL(DCPP_UTILS, 2)
#define DCPP_UTILS_CONV_UTF8_TO_WIDE		DCPP_SET_CALL(DCPP_UTILS, 3)
#define DCPP_UTILS_CONV_WIDE_TO_UTF8		DCPP_SET_CALL(DCPP_UTILS, 4)
#define DCPP_UTILS_CONV_ACP_TO_UTF8			DCPP_SET_CALL(DCPP_UTILS, 5)
#define DCPP_UTILS_CONV_UTF8_TO_ACP			DCPP_SET_CALL(DCPP_UTILS, 6)
#define DCPP_UTILS_FREE_LINKED_MAP			DCPP_SET_CALL(DCPP_UTILS, 7)



//#undef DCPP_SET_EVENT
//#undef DCPP_SET_CALL

#endif // DCPP_PLUGIN_EVENTS

/**
 * @file
 * $Id$
 */
