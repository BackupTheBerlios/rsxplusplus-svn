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

#ifndef UPDATE_MANAGER_LISTENER_H
#define UPDATE_MANAGER_LISTENER_H

namespace dcpp {

namespace VersionInfo {
	struct Client {
		string version;
		string url;
		string message;
		int svnbuild;
		vector<double> badVersions;
		double veryOldVersion;
	};

	struct Profiles {
		string profileName;
		string profileUrl;
		StringPair clientProfile;
		StringPair myInfoProfile;
		StringPair ipWatchProfile;
	};
}

class UpdateManagerListener {
public:
	template<int I>	struct X { enum { TYPE = I };  };

	typedef X<0> VersionUpdated;

	virtual void on(VersionUpdated, const VersionInfo::Client&, const VersionInfo::Profiles&) throw() { };
};
} // namespace dcpp
#endif

/**
 * @file
 * $Id: UpdateManagerListener.h 230 2011-01-30 18:15:33Z adrian_007 $
 */