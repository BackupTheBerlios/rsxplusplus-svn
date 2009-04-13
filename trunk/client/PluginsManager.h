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

#ifndef RSXPLUSPLUS_PLUGINS_MANAGER
#define RSXPLUSPLUS_PLUGINS_MANAGER

#include "Singleton.h"
#include "CriticalSection.h"

#include "sdk/dcpp.h"
#include "sdk/events.h"

namespace dcpp {
class Plugin;
class Client;

class PluginsManager : public Singleton<PluginsManager> {
public:
	PluginsManager();

	void init(void (*f)(void*, const tstring&), void* p);
	void load();
	void unload();

	inline int plugEvent(int type, void* p1, void* p2, void* p3, bool toAll = true) {
		if(toAll)
			return plugEventAll(type, p1, p2, p3);
		return plugEventBreakAtFirst(type, p1, p2, p3);
	}

	void getPluginsInfo(std::list<DCPP_PLUG_INFO*>& p);

private:
	static void debugDummy(const char*, ...) { }
	static void* callFunc(int type, void* p1, void* p2, void* p3);
	int plugEventAll(int type, void* p1, void* p2, void* p3);
	int plugEventBreakAtFirst(int type, void* p1, void* p2, void* p3);

	CriticalSection cs;
	DCPP_FUNCTIONS* dcpp_func;

	typedef vector<Plugin*> Plugins;
	Plugins plugins;
};
} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */
