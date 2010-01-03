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

#ifndef RSXPLUSPLUS_IP_MANAGER_H
#define RSXPLUSPLUS_IP_MANAGER_H

#include "../client/Singleton.h"
#include "../client/CriticalSection.h"
#include "../client/Util.h"
#include "../client/SettingsManager.h"

#include "HTTPDownloadManager.h"

namespace dcpp {

class IPWatch {
public:
	typedef IPWatch* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	IPWatch() { };
	IPWatch(int aMode, const string& aPattern, int aTask, int aAction, bool aDisplay, const string& aCheat, int mType, const string& aIsp)
		throw() : mode(aMode), pattern(aPattern), task(aTask), action(aAction), displayCheat(aDisplay), cheat(aCheat), matchType(mType), isp(aIsp) { };

	GETSET(int, mode, Mode);
	GETSET(string, pattern, Pattern);
	GETSET(int, task, Task);
	GETSET(int, action, Action);
	GETSET(bool, displayCheat, DisplayCheat);
	GETSET(string, cheat, Cheat);
	GETSET(int, matchType, MatchType);
	GETSET(string, isp, Isp);
};

class SimpleXML;
class IpManager : public Singleton<IpManager> {
public:
	void load() { WatchLoad(); }
	void WatchLoad();
	void WatchSave();
	void reloadIpWatch();

	IPWatch* addWatch(int m, const string& pat, int task, int act, bool disp, const string& cheat, int mt, const string& i) {
		IPWatch* ipw = new IPWatch(m, pat, task, act, disp, cheat, mt, i);
		ipwatch.push_back(ipw);
		return ipw;
	}

	void getWatch(unsigned int index, IPWatch &ipw) {
		if(ipwatch.size() > index)
			ipw = *ipwatch[index];
	}

	void updateWatch(unsigned int index, IPWatch &ipw) {
		*ipwatch[index] = ipw;
	}

	void removeWatch(unsigned int index) {
		if(ipwatch.size() > index)
			ipwatch.erase(ipwatch.begin() + index);
	}

	IPWatch::List& getWatch() { 
		Lock l(cs);
		return ipwatch; 
	}

	GETSET(string, ipWatchVersion, IpWatchVersion);

	//@todo make possible to use it in network page as well
	void UpdateExternalIp() {
		HTTPDownloadManager::getInstance()->addRequest(boost::bind(&IpManager::onIPUpdate, this, _1, _2), "http://checkip.dyndns.org/", false);
	}
private:
	friend class Singleton<IpManager>;

	IpManager();
	~IpManager();

	CriticalSection cs;
	IPWatch::List ipwatch;

	void loadWatch(SimpleXML& aXml);
	void clearWatchList() {
		Lock l(cs);
		for(IPWatch::Iter j = ipwatch.begin(); j!= ipwatch.end(); ++j) {
			IPWatch* ipw = *j;
			if(ipw != NULL) {
				delete ipw;
				ipw = NULL;
			}
		}
		ipwatch.clear();
	}

	void onIPUpdate(string buf, bool isFailed);
};
} // namespace dcpp
#endif //RSXPLUSPLUS_IP_MANAGER_H

/**
 * @file
 * $Id$
 */
