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

///////////////////////////////////////////////////////////////////////////
/**
	Type: Event
	Description: Core is loading
**/
#define DCPP_INIT_OPEN						1
/**
	Type: Event
	Description: Core is preparing for close
**/
#define DCPP_INIT_CLOSE						2
/**
	Type: Event
	Description: Core is loaded
**/
#define DCPP_CORE_STARTED					3
///////////////////////////////////////////////////////////////////////////
/**
	Type: Value
	Description: Return if want to drop the event by the core
**/
#define DCPP_DROP_EVENT						1
/**
	Type: Value
	Description: Return if event not handled/want to pass trought
**/
#define DCPP_PROCESS_EVENT					0
///////////////////////////////////////////////////////////////////////////
/**
	Type: None
	Description: None
**/
#define DCPP_HUB							100
#define DCPP_USER							200
#define DCPP_CONN							300
#define DCPP_ACTION							400
#define DCPP_CONFIG							500
#define DCPP_CORE							600
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	Type:
	Description:
**/
#define DCPP_HUB_OPENED						DCPP_HUB + 1
/**
	Type:
	Description:
**/
#define DCPP_HUB_CONNECTED					DCPP_HUB + 2
/**
	Type:
	Description:
**/
#define DCPP_HUB_DISCONNECTED				DCPP_HUB + 3
/**
	Type:
	Description:
**/
#define DCPP_HUB_CLOSED						DCPP_HUB + 4
/**
	Type:
	Description:
**/
#define DCPP_HUB_MSG_IN						DCPP_HUB + 5
/**
	Type:
	Description:
**/
#define DCPP_HUB_MSG_OUT					DCPP_HUB + 6
/**
	Type:
	Description:
**/
#define DCPP_HUB_PRIVATE_MSG_IN				DCPP_HUB + 7		//@todo
/**
	Type:
	Description:
**/
#define DCPP_HUB_PRIVATE_MSG_OUT			DCPP_HUB + 8		//@todo
/**
	Type:
	Description:
**/
#define DCPP_HUB_FIELD_GET					DCPP_HUB + 9
/**
	Type:
	Description:
**/
#define DCPP_HUB_FIELD_SET					DCPP_HUB + 10
/**
	Type:
	Description:
**/
#define DCPP_HUB_FIELD_IS_SET				DCPP_HUB + 11
/**
	Type:
	Description:
**/
#define DCPP_HUB_FIELD_MY_GET				DCPP_HUB + 12
/**
	Type:
	Description:
**/
#define DCPP_HUB_FIELD_MY_SET				DCPP_HUB + 13
/**
	Type:
	Description:
**/
#define DCPP_HUB_FIELD_MY_IS_SET			DCPP_HUB + 14
/**
	Type:
	Description:
**/
#define DCPP_HUB_SEND_SOCKET				DCPP_HUB + 15
/**
	Type:
	Description:
**/
#define DCPP_HUB_SEND_MESSAGE				DCPP_HUB + 16
/**
	Type:
	Description:
**/
#define DCPP_HUB_SEND_USER_CMD				DCPP_HUB + 17
/**
	Type:
	Description:
**/
#define DCPP_HUB_OPEN						DCPP_HUB + 18
/**
	Type:
	Description:
**/
#define DCPP_HUB_CLOSE						DCPP_HUB + 19
/**
	Type:
	Description:
**/
#define DCPP_HUB_REDIRECT					DCPP_HUB + 20
/**
	Type:
	Description:
**/
#define DCPP_HUB_ADD_CHAT_LINE				DCPP_HUB + 21
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	Type:
	Description:
**/
#define DCPP_USER_CONNECTED					DCPP_USER + 1
/**
	Type:
	Description:
**/
#define DCPP_USER_DISCONNECTED				DCPP_USER + 2
/**
	Type:
	Description:
**/
#define DCPP_USER_FIELD_GET					DCPP_USER + 3
/**
	Type:
	Description:
**/
#define DCPP_USER_FIELD_SET					DCPP_USER + 4
/**
	Type:
	Description:
**/
#define DCPP_USER_FIELD_IS_SET				DCPP_USER + 5
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	Type:
	Description:
**/
#define DCPP_ACT_LOG_MSG					DCPP_ACTION + 1
/**
	Type:
	Description:
**/
#define DCPP_ACT_FORMAT_PARAMS				DCPP_ACTION + 2			//#free
/**
	Type:
	Description:
**/
#define DCPP_ACT_CONV_STR_TO_T				DCPP_ACTION + 3			//#free
/**
	Type:
	Description:
**/
#define DCPP_ACT_CONV_STR_FROM_T			DCPP_ACTION + 4			//#free
/**
	Type:
	Description:
**/
#define DCPP_ACT_LUA_INIT					DCPP_ACTION + 5
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	Type:
	Description:
**/
#define DCPP_CFG_GET						DCPP_CONFIG + 1
/**
	Type:
	Description:
**/
#define DCPP_CFG_SET						DCPP_CONFIG + 2
/**
	Type:
	Description:
**/
#define DCPP_CFG_GET_CORE					DCPP_CONFIG + 3
/**
	Type:
	Description:
**/
#define DCPP_CFG_SET_CORE					DCPP_CONFIG + 4
/**
	Type:
	Description:
**/
#define DCPP_CFG_CHANGED					DCPP_CONFIG + 5
///////////////////////////////////////////////////////////////////////////

#endif // DCPP_PLUGIN_EVENTS

/**
 * @file
 * $Id$
 */
