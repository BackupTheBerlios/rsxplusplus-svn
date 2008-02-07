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
class IpManager : public Singleton<IpManager>, private HttpConnectionListener {
public:
	void load() { WatchLoad(); }
	void WatchLoad(const string& p = Util::emptyString);
	void WatchSave();
	void reloadIpWatch(const string& filePath);

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
	void UpdateExternalIp() { c->addListener(this); c->downloadFile("http://checkip.dyndns.org/"); }
private:
	IpManager();
	~IpManager();

	mutable CriticalSection cs;
	IPWatch::List ipwatch;
	friend class Singleton<IpManager>;

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
		downBuf += string((const char*)buf, len);
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
