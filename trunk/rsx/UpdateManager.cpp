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

#include "UpdateManager.h"

UpdateManager::UpdateManager() : c(NULL), current(0), working(false) { 
	c = new HttpConnection;
}

UpdateManager::~UpdateManager() {
	if(c) {
		c->removeListeners();
		delete c;
		c = NULL;
	}
	
	items.clear();
}

void UpdateManager::downloadFile(int _id, const string& aUrl) {
	Lock l(cs);
	UpdateItems::iterator i = items.find(_id);
	if(i != items.end()) {
		return;
	}

	working = !items.empty();
	items.insert(make_pair(_id, aUrl));

	if(!working) {
		startDownload();
	}
}

void UpdateManager::startDownload() {
	Lock l(cs);
	current = 0;
	downBuf = Util::emptyString;
	working = true;
	UpdateItems::const_iterator i = items.begin();
	if(i != items.end() && c) {
		current = i->first;
		c->addListener(this);
		c->downloadFile(i->second);
	}
}

void UpdateManager::on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
	{
		Lock l(cs);
		items.erase(current);
	}
	conn->removeListener(this);
	working = false;
	fire(UpdateManagerListener::Complete(), downBuf, current);
	startDownload();
}

void UpdateManager::on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw() {
	{
		Lock l(cs);
		items.erase(current);
	}
	conn->removeListener(this);
	working = false;
	fire(UpdateManagerListener::Failed(), aLine, current);
	startDownload();
}
