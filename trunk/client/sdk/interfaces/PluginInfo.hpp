/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#ifndef DCPP_INTERFACES_PLUGIN_INFO_HPP
#define DCPP_INTERFACES_PLUGIN_INFO_HPP

namespace dcpp {
	namespace interfaces {
		struct PluginInfo {
			const char*			name;			// Name of the plugin
			const char*			guid;			// GUID, set if only one instance is allowed
			const char*			author;			// Author
			const char*			description;	// Short description of the plugin
			const char*			website;		// Website of the plugin
			uint64_t			version;		// Plugin's version
			uint64_t			sdkVersion;		// SDK's version used to compile plugin, set to SDK_VERSION
		};
	}
}

#endif
