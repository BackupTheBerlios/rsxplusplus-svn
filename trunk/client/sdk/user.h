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

#ifndef DCPP_SDK_USER
#define DCPP_SDK_USER

#define DCPP_EVENT_USER "User/"
#define DCPP_CALL_USER "User/"

#define DCPP_USER_FIELD_NICK			"NI"
#define DCPP_USER_FIELD_DESCRIPTION		"DE"
#define DCPP_USER_FIELD_EMAIL			"EM"
#define DCPP_USER_FIELD_CONNECTION		"US"
#define DCPP_USER_FIELD_IP4				"I4"

#define DCPP_CALL_USER_GET_FIELD(x)		"User/Identity/Get/" x
#define DCPP_CALL_USER_SET_FIELD(x)		"User/Identity/Set/" x
#define DCPP_CALL_USER_GET_HUB_OBJECT	"User/GetHubObject"
#define DCPP_CALL_USER_GET_PARAMS		"User/GetParams"

#define DCPP_EVENT_USER_CONNECTED		"User/Connected"
#define DCPP_EVNET_USER_DISCONNECTED	"User/Disconnected"

#endif

/**
 * @file
 * $Id$
 */
