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

#ifndef IP_MANAGER_H
#define IP_MANAGER_H

#include "../client/Singleton.h"
#include "../client/CriticalSection.h"
#include "../client/Util.h"

#include "../client/HttpConnection.h"
#include "../client/SimpleXML.h"
#include "../client/SettingsManager.h"


class Isp {
public:
	typedef Isp* Ptr;
	typedef map<uint32_t, Ptr> List;
	typedef List::iterator Iter;

	Isp() { };
	Isp(const string aISP, uint32_t aLower, bool aBad) : isp(aISP), lower(aLower), bad(aBad) { };

	GETSET(string, isp, Isp);
	GETSET(uint32_t, lower, Lower);
	GETSET(bool, bad, Bad);
};

class IPWatch {
public:
	typedef IPWatch* Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;

	IPWatch() { };
	IPWatch(const string& aIp, int aAction, int aActionCommand, bool aDisplay, const string& aCheat, bool regexp)
		throw() : ip(aIp), action(aAction), actionCommand(aActionCommand), displayCheat(aDisplay), cheat(aCheat), useRegExp(regexp) { };

	GETSET(string, ip, Ip);
	GETSET(int, action, Action);
	GETSET(int, actionCommand, ActionCommand);
	GETSET(bool, displayCheat, DisplayCheat);
	GETSET(string, cheat, Cheat);
	GETSET(bool, useRegExp, UseRegExp)
};

class SimpleXML;

class IpManager : public Singleton<IpManager>, private HttpConnectionListener {
public:
	void reloadISPs() { clearISPList(); loadISPs(); }
	void load() { loadISPs(); WatchLoad(); }
	void loadISPs();
	void saveISPs();
	void WatchLoad();
	void WatchSave();

	Isp::Ptr& getISP(const string& IP);
	bool isBadRange(const uint32_t aKey);

	void insertISP(const uint32_t high, const uint32_t low, const string& isp, const bool bad) {
		Lock l(ics);
		ispList.insert(make_pair(high, new Isp(isp, low, bad)));
	}

	void removeISP(const uint32_t aKey) {
		Lock l(ics);
		Isp::Iter j = ispList.find(aKey);
		if(j != ispList.end()) {
			delete j->second;
			ispList.erase(aKey);
		}
	}

	void clearISPList() {
		Lock l(ics);
		for(Isp::Iter j = ispList.begin(); j != ispList.end(); ++j) {
			delete j->second;
		}
		ispList.clear();
	}

	Isp::List& getIspList() {
		Lock l(ics);
		reloadISPs();
		return ispList; 
	}

	IPWatch* addWatch(const string& ip, int act, int actCmd, bool disp, const string& cheat, bool ur) {
		IPWatch* ipw = new IPWatch(ip, act, actCmd, disp, cheat, ur);
		ipwatch.push_back(ipw);
		return ipw;
	}

	IPWatch* getWatch(unsigned int index, IPWatch &ipw) {
		if(ipwatch.size() > index)
			ipw = *ipwatch[index];
		return NULL;
	}

	IPWatch* updateWatch(unsigned int index, IPWatch &ipw) {
		*ipwatch[index] = ipw;
		return NULL;
	}

	IPWatch* removeWatch(unsigned int index) {
		if(ipwatch.size() > index)
			ipwatch.erase(ipwatch.begin() + index);
		return NULL;
	}

	IPWatch::List& getWatch() { 
		Lock l(cs);
		return ipwatch; 
	}
	//@todo make possible to use it in network page as well
	void UpdateExternalIp() { c->addListener(this); c->downloadFile("http://checkip.dyndns.org/"); }

private:
	friend class Singleton<IpManager>;

	IpManager();
	~IpManager();

	mutable CriticalSection cs, ics;

	Isp::List ispList;
	IPWatch::List ipwatch;
	static Isp::Ptr unknown;

	void loadISPs(SimpleXML& aXml);
	void loadWatch(SimpleXML& aXml);

	void clearWatchList() {
		Lock l(cs);
		for(IPWatch::Iter j = ipwatch.begin(); j!= ipwatch.end(); ++j) {
			delete *j;
		}
		ipwatch.clear();
	}

	//update external ip
	HttpConnection* c;
	string downBuf;

	void on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
		downBuf = string((const char*)buf, len);
	}

	void on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
		conn->removeListener(this);
		if(!downBuf.empty()) {
			SimpleXML xml;
			xml.fromXML(downBuf);
			if(xml.findChild("html")) {
				xml.stepIn();
				if(xml.findChild("body")) {
					string x = xml.getChildData().substr(20);
					if(Util::isPrivateIp(x)) {
						SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
					}
					SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, x);				
				} else {
					SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, Util::getLocalIp());
				}
			}
		}
	}

	void on(HttpConnectionListener::Failed, HttpConnection* conn, const string& /*aLine*/) throw() {
		conn->removeListener(this);
		if(!SETTING(NO_IP_OVERRIDE)) {
			if(Util::isPrivateIp(Util::getLocalIp())) {
				SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
			}
			SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, Util::getLocalIp());
		}
	}
};
#endif //IP_MANAGER_H
