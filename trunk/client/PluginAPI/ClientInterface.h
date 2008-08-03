/*
 * Copyright (C) 2007-2008 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPLUSPLUS_CLIENT_INTERFACE
#define RSXPLUSPLUS_CLIENT_INTERFACE

namespace dcpp {
// hub interface class

class iOnlineUser;
//class rString;

class iClient {
public:
	/** send message to hub **/
	virtual void __cdecl p_hubMessage(const dcpp::rString& aMsg, bool thirdPerson = false) = 0;

	/** add message to chat window (visible only for user) **/
	virtual void __cdecl p_addHubLine(const dcpp::rString& aMsg, int mType = 0) = 0;

	/** send raw message to hub **/
	virtual void __cdecl p_sendUserCmd(const dcpp::rString& aUserCmd) = 0;

	/** get/set ADC-like field value from hub identity class **/
	virtual dcpp::rString __cdecl p_getField(const char* name) = 0;
	virtual void __cdecl p_setField(const char* name, const dcpp::rString& value) = 0;

	/** get ADC-like field value from user identity class in this hub **/
	virtual dcpp::rString __cdecl p_getMyField(const char* name) = 0;
	virtual void __cdecl p_setMyField(const char* name, const dcpp::rString& value) = 0;

	/** get Hub Setting, setting name is 4-letter **/
	virtual dcpp::rString __cdecl p_getHubSetting(const char* name) = 0;
	virtual void __cdecl p_setHubSetting(const char* name, const dcpp::rString& value) = 0;

	/** get exact hub address **/
	virtual dcpp::rString __cdecl p_getHubUrl() = 0;

	/** get pointer to iOnlineUser interface class by nick **/
	virtual dcpp::iOnlineUser* __cdecl p_getUserByNick(const dcpp::rString& aNick) = 0;

	/** lock client using CriticalSection **/
	virtual void __cdecl p_lock() = 0;

	/** unlock client **/
	virtual void __cdecl p_unlock() = 0;
};
} // namespace dcpp

#endif // RSXPLUSPLUS_CLIENT_INTERFACE

/**
 * @file
 * $Id$
 */
