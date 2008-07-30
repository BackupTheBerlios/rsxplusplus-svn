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

#ifndef RSXPLUSPLUS_PLUGIN_INTERFACE
#define RSXPLUSPLUS_PLUGIN_INTERFACE

namespace dcpp {
// plugin interface, called on event in core
// override needed functions only

class iClient;
class iOnlineUser;
class iUserConnection;
class rString;

class iPlugin {
public:
	virtual bool __cdecl onIncomingMessage(iClient*, const rString&)					{ return false; }
	virtual bool __cdecl onOutgoingMessage(iClient*, const rString&)					{ return false; }

	virtual bool __cdecl onIncomingPM(iOnlineUser*, const rString&)						{ return false; }
	virtual bool __cdecl onOutgoingPM(iOnlineUser*, const rString&)						{ return false; }

	virtual bool __cdecl onHubConnected(iClient*)										{ return false; }
	virtual void __cdecl onHubDisconnected(iClient*)									{ return; }

	virtual void __cdecl onUserConnected(iOnlineUser*)									{ return; }
	virtual void __cdecl onUserDisconnected(iOnlineUser*)								{ return; }

	virtual bool __cdecl onUserConnectionIn(iUserConnection*, const rString&)			{ return false; }
	virtual bool __cdecl onUserConnectionOut(iUserConnection*, const rString&)			{ return false; }

	virtual void __cdecl onToolBarClick()												{ return; }
	virtual void __cdecl onSettingLoad()												{ return; }
};
} // namespace dcpp

#endif // RSXPLUSPLUS_PLUGIN_INTERFACE

/**
 * @file
 * $Id$
 */
