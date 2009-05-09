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

#define DCPP_INIT_OPEN						1
#define DCPP_INIT_CLOSE						2
#define DCPP_CORE_STARTED					3

#define DCPP_DROP_EVENT						1
#define DCPP_PROCESS_EVENT					0

#define DCPP_HUB_EVENT						100
#define DCPP_USER_EVENT						200
#define DCPP_CONN_EVENT						300
#define DCPP_ACTION							400
#define DCPP_CONFIG							500

#define DCPP_HUB_OPENED						DCPP_HUB_EVENT + 1
#define DCPP_HUB_CONNECTED					DCPP_HUB_EVENT + 2
#define DCPP_HUB_DISCONNECTED				DCPP_HUB_EVENT + 3
#define DCPP_HUB_CLOSED						DCPP_HUB_EVENT + 4
#define DCPP_HUB_MSG_IN						DCPP_HUB_EVENT + 5
#define DCPP_HUB_MSG_OUT					DCPP_HUB_EVENT + 6
#define DCPP_HUB_PRIVATE_MSG_IN				DCPP_HUB_EVENT + 7		//@todo
#define DCPP_HUB_PRIVATE_MSG_OUT			DCPP_HUB_EVENT + 8		//@todo
#define DCPP_HUB_FIELD_GET					DCPP_HUB_EVENT + 9
#define DCPP_HUB_FIELD_SET					DCPP_HUB_EVENT + 10
#define DCPP_HUB_FIELD_IS_SET				DCPP_HUB_EVENT + 11
#define DCPP_HUB_FIELD_MY_GET				DCPP_HUB_EVENT + 12
#define DCPP_HUB_FIELD_MY_SET				DCPP_HUB_EVENT + 13
#define DCPP_HUB_FIELD_MY_IS_SET			DCPP_HUB_EVENT + 14
#define DCPP_HUB_SEND_SOCKET				DCPP_HUB_EVENT + 15
#define DCPP_HUB_SEND_MESSAGE				DCPP_HUB_EVENT + 16
#define DCPP_HUB_SEND_USER_CMD				DCPP_HUB_EVENT + 17
#define DCPP_HUB_OPEN						DCPP_HUB_EVENT + 18
#define DCPP_HUB_CLOSE						DCPP_HUB_EVENT + 19
#define DCPP_HUB_REDIRECT					DCPP_HUB_EVENT + 20
#define DCPP_HUB_ADD_CHAT_LINE				DCPP_HUB_EVENT + 21

#define DCPP_USER_CONNECTED					DCPP_USER_EVENT + 1
#define DCPP_USER_DISCONNECTED				DCPP_USER_EVENT + 2
#define DCPP_USER_FIELD_GET					DCPP_USER_EVENT + 3
#define DCPP_USER_FIELD_SET					DCPP_USER_EVENT + 4
#define DCPP_USER_FIELD_IS_SET				DCPP_USER_EVENT + 5

#define DCPP_ACT_LOG_MSG					DCPP_ACTION + 1
#define DCPP_ACT_FORMAT_PARAMS				DCPP_ACTION + 2			//#free
#define DCPP_ACT_CONV_STR_TO_T				DCPP_ACTION + 3			//#free
#define DCPP_ACT_CONV_STR_FROM_T			DCPP_ACTION + 4			//#free
#define DCPP_ACT_LUA_INIT					DCPP_ACTION + 5

#define DCPP_CFG_GET						DCPP_CONFIG + 1
#define DCPP_CFG_SET						DCPP_CONFIG + 2
#define DCPP_CFG_GET_CORE					DCPP_CONFIG + 3
#define DCPP_CFG_SET_CORE					DCPP_CONFIG + 4
#define DCPP_CFG_CHANGED					DCPP_CONFIG + 5

#endif // DCPP_PLUGIN_EVENTS

/**
 * @file
 * $Id$
 */
