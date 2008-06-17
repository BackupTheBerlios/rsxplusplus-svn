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

#ifndef RSXPLUSPLUS_HTTP_DOWNLOAD_MANAGER_H
#define RSXPLUSPLUS_HTTP_DOWNLOAD_MANAGER_H

#include "../client/Singleton.h"
#include "../client/TimerManager.h"

#include "HTTPDownloadItem.h"

namespace dcpp {

class HTTPDownloadManager : public Singleton<HTTPDownloadManager>, private TimerManagerListener {
public:
	HTTPDownloadManager() {
		TimerManager::getInstance()->addListener(this);
	}
	~HTTPDownloadManager() {
		TimerManager::getInstance()->removeListener(this);
	}

	bool addRequest(const HTTPDownloadItem::CallBack& callBack, const string& aUrl);
private:
	typedef vector<HTTPDownloadItem*> Requests;

	void on(TimerManagerListener::Second, uint64_t) throw();

	Requests requests;
	CriticalSection cs;
};
} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */