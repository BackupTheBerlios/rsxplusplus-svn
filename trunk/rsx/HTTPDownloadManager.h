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

#ifndef RSXPLUSPLUS_HTTP_DOWNLOAD_MANAGER_H
#define RSXPLUSPLUS_HTTP_DOWNLOAD_MANAGER_H

#include "../client/Singleton.h"
#include "../client/TimerManager.h"
#include "../client/HttpConnection.h"

namespace dcpp {

class HTTPDownloadManager : public Singleton<HTTPDownloadManager>, private TimerManagerListener, private HttpConnectionListener {
public:
	typedef boost::function<void(string, bool)> CallBack;

	HTTPDownloadManager();
	~HTTPDownloadManager();

	bool addRequest(const CallBack& callBack, const string& aUrl, bool useCoral = true);
private:
	struct Item {
		CallBack cb;
		string url;
		bool coral;
	};

	typedef std::deque<Item> ItemsQueue;

	void on(HttpConnectionListener::Complete, HttpConnection*, const string&, bool) throw();
	void on(HttpConnectionListener::Failed, HttpConnection*, const string& aLine) throw();
	void on(HttpConnectionListener::Data, HttpConnection*, const uint8_t* buf, size_t len) throw() {
		downBuf.append((char*)buf, len);
	}

	CriticalSection cs;
	ItemsQueue queue;

	HttpConnection* c;
	bool idle;
	string downBuf;

	void on(TimerManagerListener::Second, uint64_t) throw();
};
} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */