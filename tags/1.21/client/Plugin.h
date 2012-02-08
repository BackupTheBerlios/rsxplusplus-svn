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

#ifndef RSXPLUSPLUS_PLUGIN
#define RSXPLUSPLUS_PLUGIN

#include "sdk/interfaces/Core.hpp"
#include "sdk/interfaces/PluginInfo.hpp"

namespace dcpp {

class Plugin {
public:
	typedef int (__stdcall * PluginLoad)(dcpp::interfaces::Core*, void*);
	typedef int (__stdcall * PluginUnload)();

	Plugin(HINSTANCE h) : handle(h) { }

	PluginLoad pluginLoad;
	PluginUnload pluginUnload;

	dcpp::interfaces::PluginInfo* info;
	HINSTANCE handle;
};

} // namespace dcpp
#endif // RSXPLUSPLUS_PLUGIN
