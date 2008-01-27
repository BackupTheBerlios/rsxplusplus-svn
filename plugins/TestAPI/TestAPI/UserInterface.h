/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

class iClient;
class iUser {
public:
	virtual string iGet(const char* name) const = 0;
	virtual string iGetNick() const = 0;
	virtual void sendPM(const std::string& aMsg) = 0;
	virtual iClient* getUserClient() = 0;
};
#endif

/**
 * @file
 * $Id: PluginAPI.h 32 2007-09-16 20:49:44Z adrian_007 $
 */