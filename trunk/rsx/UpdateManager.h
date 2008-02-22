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

#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#include "../client/Singleton.h"
#include "../client/HttpConnection.h"
#include "../client/CriticalSection.h"

#include "UpdateManagerListener.h"

class UpdateManager : public Singleton<UpdateManager>, public Speaker<UpdateManagerListener>, private HttpConnectionListener {
public:

	enum {
		CLIENT = 0,
		MYINFO = 1,
		IPWATCH = 2,
		VERSION = 3,
		PROFILE_VERSION = 4
	};

	UpdateManager();
	~UpdateManager();	

	void downloadFile(int _id, const string& aUrl);

private:
	friend class Singleton<UpdateManager>;
	typedef unordered_map<int, string> UpdateItems;
	bool working;

	void startDownload();

	void on(HttpConnectionListener::Complete, HttpConnection*, const string&) throw();
	void on(HttpConnectionListener::Failed, HttpConnection*, const string& aLine) throw();
	void on(HttpConnectionListener::Data, HttpConnection*, const uint8_t* buf, size_t len) throw() {
		downBuf.append((char*)buf, len);
	}

	int current;
	CriticalSection cs;
	HttpConnection* c;
	string downBuf;
	UpdateItems items;
};
#endif