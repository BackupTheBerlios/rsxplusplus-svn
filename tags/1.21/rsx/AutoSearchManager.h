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

#ifndef AUTO_SEARCH_MANAGER_H
#define AUTO_SEARCH_MANAGER_H

#include "../client/TimerManager.h"
#include "../client/SearchManager.h"
#include "../client/SearchManagerListener.h"

#include "../client/Thread.h"
#include "../client/Singleton.h"
#include "../client/Util.h"

namespace dcpp {

class Autosearch {
public:
	typedef Autosearch* Ptr;
	typedef vector<Ptr> List;

	Autosearch() { };
	Autosearch(bool aEnabled, const string& aSearchString, int aFileType, int aAction, int aActionCommand, bool aDisplay, const string& aCheat)
		throw() : enabled(aEnabled), searchString(aSearchString), fileType(aFileType), action(aAction), raw(aActionCommand), displayCheat(aDisplay), cheat(aCheat) { };

	GETSET(bool, enabled, Enabled);
	GETSET(string, searchString, SearchString);
	GETSET(int, action, Action);
	GETSET(int, fileType, FileType);
	GETSET(int, raw, Raw);
	GETSET(bool, displayCheat, DisplayCheat);
	GETSET(string, cheat, Cheat);
};

class SimpleXML;

class AutoSearchManager : public Singleton<AutoSearchManager>, private TimerManagerListener, private SearchManagerListener {
public:
	AutoSearchManager();
	~AutoSearchManager();

	Autosearch* addAutosearch(bool en, const string& ss, int ft, int act, int actCmd, bool disp, const string& cheat) {
		Autosearch* ipw = new Autosearch(en, ss, ft, act, actCmd, disp, cheat);
		as.push_back(ipw);
		return ipw;
	}

	void removeAutosearch(unsigned int index) {
		if(as.size() > index) {
			Autosearch::List::iterator i = as.begin() + index;
			Autosearch* a = (*i);
			as.erase(i);
			delete a;
			a = NULL;
		}
	}

	Autosearch::List& getAutosearch() { 
		Lock l(cs);
		return as; 
	}

	GETSET(string, version, Version);

	void AutosearchLoad();
	void AutosearchSave();
private:
	friend class Singleton<AutoSearchManager>;
	void stopTaskThread();

	GETSET(uint16_t, time, Time);

	CriticalSection cs;

	Autosearch::List vs; //valid searches
	Autosearch::List as; //all searches
	StringList allowedHubs;

	void loadAutosearch(SimpleXML& aXml);
	void removeRegExpFromSearches();
	void getAllowedHubs();

	void on(SearchManagerListener::SR, const SearchResultPtr& sr) throw();
	void on(TimerManagerListener::Minute, uint64_t aTick) throw();
	void addResultToQueue(const SearchResultPtr& sres, Autosearch* a);

	bool endOfList;
	int curPos;
	uint16_t recheckTime;
	string curSearch;
	set<UserPtr> users;
};
}; // namespace dcpp
#endif