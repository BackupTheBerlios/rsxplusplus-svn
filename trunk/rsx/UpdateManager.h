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

#ifndef RSXPLUSPLUS_UPDATE_MANAGER_H
#define RSXPLUSPLUS_UPDATE_MANAGER_H

#include "../client/Singleton.h"
#include "../client/HttpConnection.h"
#include "../client/CriticalSection.h"
#include "../client/TimerManager.h"

#include "UpdateManagerListener.h"

namespace dcpp {

class UpdateManager : public Singleton<UpdateManager>, public Speaker<UpdateManagerListener>, private TimerManagerListener {
public:
	UpdateManager();
	~UpdateManager();	

	void getCacheInfo(VersionInfo::Client& c, VersionInfo::Profiles& p) {
		c = clientCache;
		p = profilesCache;
	}

	void runUpdate();
	std::string getLatestVersion() const {
		return clientCache.version;
	}

private:
	void resetInfo();

	friend class Singleton<UpdateManager>;

	VersionInfo::Client clientCache;
	VersionInfo::Profiles profilesCache;

	void onVersionXml(string content, bool isFailed);
	void onProfileVersionXml(string content, bool isFailed);

	void on(TimerManagerListener::Minute, uint64_t) throw();
	int minutes;

	CriticalSection cs;
};
} // namespace dcpp
#endif

/**
 * @file
 * $Id$
 */