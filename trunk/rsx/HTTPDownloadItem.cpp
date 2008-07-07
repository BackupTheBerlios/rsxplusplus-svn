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

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"

#include "HTTPDownloadItem.h"
#include "HTTPDownloadManager.h"
#include "../client/LogManager.h"

namespace dcpp {

HTTPDownloadItem::HTTPDownloadItem(const CallBack& cb) : c(new HttpConnection), _idle(false) {
	callback = cb;
	c->addListener(this);
}

HTTPDownloadItem::~HTTPDownloadItem() throw() {
	if(c) {
		c->removeListeners();
		delete c;
		c = NULL;
	}
}

void HTTPDownloadItem::on(HttpConnectionListener::Complete, HttpConnection*, const string&) throw() {
	Lock l(cs);
	c->removeListeners();
	if(!callback.empty()) {
		callback(downBuf, false);
	}
	_idle = true;
}

void HTTPDownloadItem::on(HttpConnectionListener::Failed, HttpConnection*, const string& aLine) throw() {
	Lock l(cs);
	c->removeListeners();
	if(!callback.empty()) {
		callback(aLine, true);
	}
	dcdebug("Failed on HTTP Download, reason: %s\n", aLine.c_str());
	_idle = true;
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */