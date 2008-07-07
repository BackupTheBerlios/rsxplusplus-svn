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

#include "HTTPDownloadManager.h"

namespace dcpp {

bool HTTPDownloadManager::addRequest(const HTTPDownloadItem::CallBack& callBack, const string& aUrl, bool useCoral /*= true*/) {
	Lock l(cs);
	HTTPDownloadItem* item = new HTTPDownloadItem(callBack);
	Requests::const_iterator i = std::find(requests.begin(), requests.end(), item);
	if(i != requests.end()) {
		delete item;
		item = NULL;
		return false;
	}
	requests.push_back(item);
	item->startDownload(aUrl, useCoral);
	return true;
}

void HTTPDownloadManager::on(TimerManagerListener::Second, uint64_t) throw() {
	Lock l(cs);
	for(Requests::iterator i = requests.begin(); i != requests.end();) {
		if((*i)->idle()) {
			HTTPDownloadItem* aItem = *i;
			i = requests.erase(i);
			if(aItem) {
				delete aItem;
				aItem = NULL;
			}
		} else {
			++i;
		}
	}
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */