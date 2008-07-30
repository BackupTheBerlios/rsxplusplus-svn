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

#ifndef RSXPLUSPLUS_USER_INTERFACE
#define RSXPLUSPLUS_USER_INTERFACE

namespace dcpp {
// online user interface class

class iClient;
class rString;

class iOnlineUser {
public:
	/** get/set ADC-like field value from/in user's identity class **/
	virtual dcpp::rString __cdecl p_get(const char* name) const  = 0;
	virtual void __cdecl p_set(const char* name, const dcpp::rString& value) = 0;

	/** get Client type (Op/Bot/Hub/Registered etc) **/
	virtual bool __cdecl p_isClientType(int mode) const = 0;

	/** check if user is TCP Active/Passive **/
	virtual bool __cdecl p_isTcpActive() const = 0;

	/** send private message to user **/
	virtual void __cdecl p_sendPM(const dcpp::rString& aMsg, bool thirdPerson = false) = 0;

	/** get pointer to iClient interface class where user is online **/
	virtual dcpp::iClient* __cdecl p_getUserClient() = 0;

	/** Smart Pointer Ref Counter, be careful with it **/
	/** increment reference **/
	virtual void __cdecl p_inc() = 0;

	/** decrement reference **/
	virtual void __cdecl p_dec() = 0;
};
} // namespace dcpp

#endif // RSXPLUSPLUS_USER_INTERFACE

/**
 * @file
 * $Id$
 */
