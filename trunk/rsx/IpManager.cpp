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

IpManager::IpManager() : c(NULL), ipWatchVersion("1.00") { 
	c = new HttpConnection; 
}

IpManager::~IpManager() {
	WatchSave();
	clearWatchList();
	if(c != NULL) { 
		delete c; 
		c = NULL; 
	}
}

void IpManager::reloadIpWatch(const string& filePath) {
	Lock l(cs);
	clearWatchList();
	WatchLoad(filePath);
}

void IpManager::WatchSave() {
	try {
		SimpleXML xml;

		xml.addTag("IPWatch");
		xml.stepIn();
		{
			xml.addTag("IPWatch");
			xml.stepIn();
			{
				for(IPWatch::Iter i = ipwatch.begin(); i != ipwatch.end(); ++i) {
					xml.addTag("IPWatch");
					xml.addChildAttrib("Mode", (*i)->getMode());
					xml.addChildAttrib("Pattern", (*i)->getPattern());
					xml.addChildAttrib("Task", (*i)->getTask());
					xml.addChildAttrib("Action", (*i)->getAction());
					xml.addChildAttrib("DisplayCheat", (*i)->getDisplayCheat());
					xml.addChildAttrib("Cheat", (*i)->getCheat());
					xml.addChildAttrib("MatchType", (*i)->getMatchType());
					xml.addChildAttrib("ISP", (*i)->getIsp());
				}
			}
			xml.stepOut();
			xml.addTag("RevisionInfo");
			xml.stepIn();
			{
				xml.addTag("Version", getIpWatchVersion());
			}
			xml.stepOut();
		}
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
			addWatch(aXml.getIntChildAttrib("Mode"), aXml.getChildAttrib("Pattern"), aXml.getIntChildAttrib("Task"), aXml.getIntChildAttrib("Action"),
				aXml.getBoolChildAttrib("DisplayCheat"), aXml.getChildAttrib("Cheat"), aXml.getIntChildAttrib("MatchType"), aXml.getChildAttrib("ISP"));			
		}
		aXml.stepOut();
		aXml.resetCurrentChild();
		if(aXml.findChild("RevisionInfo")) {
			aXml.stepIn();
			if(aXml.findChild("Version")) {
				setIpWatchVersion(aXml.getChildData());
				aXml.resetCurrentChild();
			}
			aXml.stepOut();
		}
	}
}

void IpManager::WatchLoad(const string& p) {
	try {
		SimpleXML xml;
		xml.fromXML(File(p.empty() ? Util::getConfigPath() + "IPWatch.xml" : p, File::READ, File::OPEN).read());
		if(xml.findChild("IPWatch")) {
			xml.stepIn();
			loadWatch(xml);
			xml.stepOut();
		}
	} catch(const Exception& e) {
		dcdebug("FavoriteManager::recentload: %s\n", e.getError().c_str());
	}	
}