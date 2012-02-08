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

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"

#include "HTTPDownloadManager.h"

namespace dcpp {

HTTPDownloadManager::HTTPDownloadManager() : c(new HttpConnection), idle(true) {
	TimerManager::getInstance()->addListener(this);
	c->addListener(this);
}

HTTPDownloadManager::~HTTPDownloadManager() {
	TimerManager::getInstance()->removeListener(this);
	if(c) {
		c->removeListener(this);
		delete c;
		c = 0;
	}
}

bool HTTPDownloadManager::addRequest(const CallBack& callBack, const string& aUrl, bool useCoral /*= true*/) {
	Lock l(cs);
	for(ItemsQueue::const_iterator i = queue.begin(); i != queue.end(); ++i) {
		if(i->url == aUrl)
			return false;
	}

	Item item = { callBack, aUrl, useCoral };
	queue.push_back(item);
	return true;
}

void HTTPDownloadManager::on(TimerManagerListener::Second, uint64_t) throw() {
	Lock l(cs);
	if(!queue.empty() && idle) {
		Item& i = queue.front();
		downBuf.clear();

		if(i.coral) {
			c->setCoralizeState(HttpConnection::CST_NOCORALIZE);
		}

		idle = false;
		c->downloadFile(i.url);
	}
}

void HTTPDownloadManager::on(HttpConnectionListener::Complete, HttpConnection*, const string&, bool) throw() {
	Lock l(cs);
	Item& i = queue.front();
	if(!i.cb.empty()) {
		i.cb(downBuf, false);
	}
	queue.pop_front();
	idle = true;
}

void HTTPDownloadManager::on(HttpConnectionListener::Failed, HttpConnection*, const string& aLine) throw() {
	Lock l(cs);
	Item& i = queue.front();
	if(!i.cb.empty()) {
		i.cb(downBuf, true);
	}
	queue.pop_front();
	idle = true;
}

} // namespace dcpp

/**
 * @file
 * $Id: HTTPDownloadManager.cpp 230 2011-01-30 18:15:33Z adrian_007 $
 */