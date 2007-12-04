/*
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

#include "../client/CriticalSection.h"
#include "../client/Singleton.h"
#include "../client/Util.h"

#define AUTOSEARCH_FILE "Autosearch.xml"

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

	void on(TimerManagerListener::Minute, uint64_t aTick) throw();
	void on(SearchManagerListener::SR, SearchResult*) throw();

	Autosearch* addAutosearch(bool en, const string& ss, int ft, int act, int actCmd, bool disp, const string& cheat) {
		Autosearch* ipw = new Autosearch(en, ss, ft, act, actCmd, disp, cheat);
		as.push_back(ipw);
		return ipw;
	}
	void getAutosearch(unsigned int index, Autosearch &ipw) {
		if(as.size() > index)
			ipw = *as[index];
	}
	void updateAutosearch(unsigned int index, Autosearch &ipw) {
		*as[index] = ipw;
	}
	void removeAutosearch(unsigned int index) {
		if(as.size() > index)
			as.erase(as.begin() + index);
	}
	Autosearch::List& getAutosearch() { 
		Lock l(acs);
		return as; 
	};

	void moveAutosearchUp(unsigned int id) {
		Lock l(acs);
		//hack =]
		if(as.size() > id) {
			swap(as[id], as[id-1]);
		}
	}

	void moveAutosearchDown(unsigned int id) {
		Lock l(acs);
		//hack =]
		if(as.size() > id) {
			swap(as[id], as[id+1]);
		}
	}

	void setActiveItem(unsigned int index, bool active) {
		Autosearch::List::iterator i = as.begin() + index;
		if(i < as.end()) {
			(*i)->setEnabled(active);
		}
	}

	void AutosearchLoad();
	void AutosearchSave();
private:
	CriticalSection cs, acs;

	Autosearch::List vs; //valid searches
	Autosearch::List as; //all searches

	void loadAutosearch(SimpleXML& aXml);

	StringList allowedHubs;
	int curPos;

	friend class Singleton<AutoSearchManager>;

	void removeRegExpFromSearches();
	void getAllowedHubs();
	string matchDirectory(const string& aFile, const string& aStrToMatch);

	GETSET(uint16_t, time, Time);

	void addToQueue(SearchResult* sr, bool pausePrio = false);

	bool endOfList;
	uint16_t recheckTime;
	string curSearch;
	set<UserPtr> users;
};
#endif