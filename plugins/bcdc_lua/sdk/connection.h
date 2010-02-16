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

#ifndef DCPP_SDK_CONNECTION
#define DCPP_SDK_CONNECTION

#define DCPP_EVENT_CONNECTION "UserConnection/"
#define DCPP_CALL_CONNECTION "UserConnection/"

#define DCPP_EVENT_CONNECTION_LINE			1

#define DCPP_CALL_CONNECTION_WRITE_LINE		"UserConnection/WriteLine"
#define DCPP_CALL_CONNECTION_DISCONNECT		"UserConnection/Disconnect"
#define DCPP_CALL_CONNECTION_SET_FLAGS		"UserConnection/SetFlags"
#define DCPP_CALL_CONNECTION_GET_FLAGS		"UserConnection/GetFlags"
#define DCPP_CALL_CONNECTION_GET_INFO		"UserConnection/GetInfo"

typedef struct {
	uint16_t		port;
	const char*		ip;
	uint8_t			secured;

	uint16_t		flags;
	uint16_t		state;
	uint16_t		slotType;
} dcppConnectionInfo;

typedef struct {
	dcpp_ptr_t		connectionPtr;

	dcpp_ptr_t		length;
	uint16_t		flags;
	const char*		line;
} dcppConnectionLine;

#endif

/**
 * @file
 * $Id: connection.h 187 2010-01-08 14:50:36Z adrian_007 $
 */
