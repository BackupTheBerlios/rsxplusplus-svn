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

#include "../client/ResourceManager.h"
#include "../client/Pointer.h"
#include "../client/SimpleXML.h"

#include "IpManager.h"
#include "RsxUtil.h"
#include "../client/rsxppSettingsManager.h"

namespace dcpp {

IpManager::IpManager() : ipWatchVersion("1.00") { 

}

IpManager::~IpManager() {
	clearWatchList();
}

void IpManager::reloadIpWatch() {
	Lock l(cs);
	clearWatchList();
	WatchLoad();
}

void IpManager::WatchSave() {
	Lock l(cs);
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
		dcdebug("IpManager::WatchSave: %s\n", e.getError().c_str());
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
		dcdebug("IpManager::WatchLoad: %s\n", e.getError().c_str());
	}	
}

void IpManager::onIPUpdate(string buf, bool isFailed) {
	if(!isFailed) {
		if(!buf.empty()) {
			try {
				SimpleXML xml;
				xml.fromXML(buf);
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
			} catch(const SimpleXMLException&) {
				//...
			}
		}
	} else {
		if(!SETTING(NO_IP_OVERRIDE)) {
			if(Util::isPrivateIp(Util::getLocalIp())) {
				SettingsManager::getInstance()->set(SettingsManager::INCOMING_CONNECTIONS, SettingsManager::INCOMING_FIREWALL_PASSIVE);
			}
			SettingsManager::getInstance()->set(SettingsManager::EXTERNAL_IP, Util::getLocalIp());
		}
	}

}
}; // namespace dcpp

/**
 * @file
 * $Id$
 */
