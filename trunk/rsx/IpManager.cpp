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
 
#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"

#include "../client/ResourceManager.h"
#include "../client/Pointer.h"

#include "IpManager.h"
#include "RsxUtil.h"
#include "rsx-settings/rsx-SettingsManager.h"

#define ISP_FILE "ISPs.xml"
Isp::Ptr IpManager::unknown = new Isp("Unknown", 0, false);

IpManager::IpManager() { 
	c = new HttpConnection; 
}

IpManager::~IpManager() {
	clearISPList();
	clearWatchList();
	if(c != NULL) { 
		delete c; 
		c = NULL; 
	}
	for_each(ipwatch.begin(), ipwatch.end(), DeleteFunction());
}

//ISP check ////////////////////////////////////////////////////////////////////////////////
//@todo completly remove isp list and extend ip watch to handle isp check ???
void IpManager::loadISPs() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + ISP_FILE, File::READ, File::OPEN).read());
		if(xml.findChild("ISP")) {
			xml.stepIn();
			loadISPs(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("HubManager::loadISPs: %s\n", e.getError().c_str());
	}
}

void IpManager::loadISPs(SimpleXML& aXml) {
	bool checkConflicts = RSXBOOLSETTING(CHECK_CONFLICTS);
	aXml.resetCurrentChild();
	string msg;
	bool conflict;
	bool write = false;
	while(aXml.findChild("ISPs")) {
		conflict = false;
		string range(aXml.getChildAttrib("Range"));
		string isp(aXml.getChildAttrib("ISP"));
		bool bad(aXml.getBoolChildAttrib("Bad"));
		uint32_t low = RsxUtil::getLowerRange(range);
		uint32_t high = RsxUtil::getUpperRange(range);
		if(checkConflicts) {
			for(Isp::Iter j = ispList.begin(); j != ispList.end(); ++j) {
				if( (j->second->getLower() <= low && j->first >= low) || (j->second->getLower() <= high && j->first >= high) ) {
					msg += STRING(ISPDLG_RANGE) + ": \r\n" + range + " " + STRING(ISP) + ": " + isp + "\r\n" + STRING(ISPDLG_CONFLICT) + " \r\n" + RsxUtil::toIP(j->second->getLower()) + '-' + RsxUtil::toIP(j->first) + " " + STRING(ISP) + ": " + j->second->getIsp() + "\r\n" + STRING(NOT_ADDED_ISP_LIST) + "\r\n\r\n";
					write = conflict = true;
				}
			}
		}
		if(!conflict) {
			insertISP(RsxUtil::getUpperRange(range), RsxUtil::getLowerRange(range), isp, bad);
		}
	}
	if(write) {
		string fname = Util::getConfigPath() + "conflicts.txt";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(msg);
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	}
}

void IpManager::saveISPs() {
	try {
		if(!ispList.empty()) {
			SimpleXML xml;

			xml.addTag("ISP");
			xml.stepIn();

			for(Isp::Iter l = ispList.begin(); l != ispList.end(); ++l) {
				xml.addTag("ISPs");
				xml.addChildAttrib("Range", static_cast<const string&>(RsxUtil::toIP(l->second->getLower()) + '-' + RsxUtil::toIP(l->first))); //stl complained about that...
				xml.addChildAttrib("ISP", l->second->getIsp());
				xml.addChildAttrib("Bad", l->second->getBad());
			}
			xml.stepOut();

			string fname = Util::getConfigPath() + ISP_FILE;

			File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
			f.write(SimpleXML::utf8Header);
			f.write(xml.toXML());
			f.close();
			File::deleteFile(fname);
			File::renameFile(fname + ".tmp", fname);
		}
	} catch(const Exception& e) {
		dcdebug("HubManager::saveISPs: %s\n", e.getError().c_str());
	}
}

Isp::Ptr& IpManager::getISP(const string& IP) {
	string::size_type a = IP.find('.');
	string::size_type b = IP.find('.', a+1);
	string::size_type c = IP.find('.', b+2);

	uint32_t ipnum = (Util::toUInt32(IP.c_str()) << 24) | 
		(Util::toUInt32(IP.c_str() + a + 1) << 16) | 
		(Util::toUInt32(IP.c_str() + b + 1) << 8) | 
		(Util::toUInt32(IP.c_str() + c + 1) );
	{
		Lock l(ics);
		Isp::Iter j = ispList.lower_bound(ipnum);
		if(j != ispList.end()) {
			if(j->second->getLower() <= ipnum) {
				return j->second;
			}
		}
	}
	return unknown;
}

bool IpManager::isBadRange(uint32_t aKey) {
	Lock l(ics);
	Isp::Iter j = ispList.find(aKey);
	if(j != ispList.end()) {
		return j->second->getBad();
	}
	return false;
}
//IP Watch ///////////////////////////////////////////////////////////////
void IpManager::WatchSave() {
	try {
		SimpleXML xml;

		xml.addTag("IPWatch");
		xml.stepIn();
		xml.addTag("IPWatch");
		xml.stepIn();

		for(IPWatch::Iter i = ipwatch.begin(); i != ipwatch.end(); ++i) {
			xml.addTag("IPWatch");
			xml.addChildAttrib("IP", (*i)->getIp());
			xml.addChildAttrib("Action", (*i)->getAction());
			xml.addChildAttrib("ActionCommand", (*i)->getActionCommand());
			xml.addChildAttrib("DisplayCheat", (*i)->getDisplayCheat());
			xml.addChildAttrib("Cheat", (*i)->getCheat());
			xml.addChildAttrib("UseRegExp", (*i)->getUseRegExp());
		}

		xml.stepOut();
		xml.stepOut();
		
		string fname = Util::getConfigPath() + "IPWatch.xml";

		File f(fname + ".tmp", File::WRITE, File::CREATE | File::TRUNCATE);
		f.write(SimpleXML::utf8Header);
		f.write(xml.toXML());
		f.close();
		File::deleteFile(fname);
		File::renameFile(fname + ".tmp", fname);
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentsave: %s\n", e.getError().c_str());
	}
}
void IpManager::loadWatch(SimpleXML& aXml){
	aXml.resetCurrentChild();
	if(aXml.findChild("IPWatch")) {
		aXml.stepIn();
		while(aXml.findChild("IPWatch")) {					
			addWatch(aXml.getChildAttrib("IP"), aXml.getIntChildAttrib("Action"), aXml.getIntChildAttrib("ActionCommand"),
				aXml.getBoolChildAttrib("DisplayCheat"), aXml.getChildAttrib("Cheat"), aXml.getBoolChildAttrib("UseRegExp"));			
		}
		aXml.stepOut();
	}
}

void IpManager::WatchLoad() {
	try {
		SimpleXML xml;
		xml.fromXML(File(Util::getConfigPath() + "IPWatch.xml", File::READ, File::OPEN).read());
		if(xml.findChild("IPWatch")) {
			xml.stepIn();
			loadWatch(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentload: %s\n", e.getError().c_str());
	}	
}