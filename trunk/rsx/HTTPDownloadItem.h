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

#ifndef RSXPLUSPLUS_HTTP_DOWNLOAD_ITEM
#define RSXPLUSPLUS_HTTP_DOWNLOAD_ITEM

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "../client/Thread.h"
#include "../client/HttpConnection.h"

namespace dcpp {

class HTTPDownloadItem : private HttpConnectionListener {
public:
	// void (string content, bool isFailed)
	typedef boost::function<void(string, bool)> CallBack;

	HTTPDownloadItem(const CallBack& cb);
	~HTTPDownloadItem() throw();

	void startDownload(const string& aUrl, bool useCoral = true) {
		if(!useCoral)
			c->setCoralizeState(HttpConnection::CST_NOCORALIZE);
		c->downloadFile(aUrl);
	}
	bool idle() const { return _idle; }
private:
	void on(HttpConnectionListener::Complete, HttpConnection*, const string&) throw();
	void on(HttpConnectionListener::Failed, HttpConnection*, const string& aLine) throw();
	void on(HttpConnectionListener::Data, HttpConnection*, const uint8_t* buf, size_t len) throw() {
		downBuf.append((char*)buf, len);
	}

	CriticalSection cs;
	HttpConnection* c;
	CallBack callback;
	string downBuf;
	bool _idle;
};

} // namespace dcpp

#endif

/**
 * @file
 * $Id$
 */