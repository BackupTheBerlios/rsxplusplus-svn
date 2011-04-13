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

#include "../client/stdinc.h"
#include "../client/DCPlusPlus.h"

#include "../client/Util.h"
#include "../client/SimpleXML.h"
#include "../client/LogManager.h"
#include "../client/version.h"

#include "../client/rsxppSettingsManager.h"

#include "UpdateManager.h"
#include "HTTPDownloadManager.h"

namespace dcpp {

UpdateManager::UpdateManager() {
	TimerManager::getInstance()->addListener(this);
	// 3-6 h
	minutes = Util::rand(180, 360);
}

UpdateManager::~UpdateManager() {
	TimerManager::getInstance()->removeListener(this);
}

void UpdateManager::runUpdate() {
	resetInfo();
	HTTPDownloadManager::getInstance()->addRequest(boost::bind(&UpdateManager::onVersionXml, this, _1, _2), VERSION_URL);
}

void UpdateManager::resetInfo() {
	Lock l(cs);

	clientCache.version = "N/A";
	clientCache.message = Util::emptyString;
	clientCache.url = Util::emptyString;
	clientCache.veryOldVersion = 0;
	clientCache.svnbuild = 0;
	clientCache.badVersions.clear();

	profilesCache.profileName = Util::emptyString;
	profilesCache.profileUrl = Util::emptyString;
	profilesCache.clientProfile.first = "N/A";
	profilesCache.clientProfile.second = Util::emptyString;
	profilesCache.myInfoProfile.first = "N/A";
	profilesCache.myInfoProfile.second = Util::emptyString;
	profilesCache.ipWatchProfile.first = "N/A";
	profilesCache.ipWatchProfile.second = Util::emptyString;
}

void UpdateManager::onVersionXml(string content, bool isFailed) {
	Lock l(cs);
	if(isFailed) {
		if(content.empty()) 
			content = "unknown exception.";
		LogManager::getInstance()->message("[Update] Error: " + content);
	} else {
		try {
			SimpleXML xml;
			xml.fromXML(content);

			if(xml.findChild("DCUpdate")) {
				xml.stepIn();
				if(xml.findChild("Version")) {
					clientCache.version = xml.getChildData();
				}
				xml.resetCurrentChild();
				if(xml.findChild("SVN")) {
					clientCache.svnbuild = Util::toInt(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("URL")) {
					clientCache.url = xml.getChildData();
				}
				xml.resetCurrentChild();
				while(xml.findChild("Message")) {
					clientCache.message += xml.getChildData();
					clientCache.message += "\r\n";
				}
				xml.resetCurrentChild();
				if(xml.findChild("VeryOldVersion")) {
					clientCache.veryOldVersion = Util::toDouble(xml.getChildData());
				}
				xml.resetCurrentChild();
				if(xml.findChild("BadVersion")) {
					xml.stepIn();
					while(xml.findChild("BadVersion")) {
						double v = Util::toDouble(xml.getChildAttrib("SVN"));
						clientCache.badVersions.push_back(v);
					}
					xml.stepOut();
				}
				xml.resetCurrentChild();
				xml.stepOut();
			}
		} catch(const Exception& e) {
			LogManager::getInstance()->message("[Update] XML Exception: " + e.getError());
		}
	}

	string profileXml = RSXPP_SETTING(PROFILE_VER_URL);
	if(profileXml.empty() == false) {
		profileXml += "profileVersion.xml";
		HTTPDownloadManager::getInstance()->addRequest(boost::bind(&UpdateManager::onProfileVersionXml, this, _1, _2), profileXml);
	}
}

void UpdateManager::onProfileVersionXml(string content, bool isFailed) {
	Lock l(cs);
	if(isFailed) {
		if(content.empty()) 
			content = "unknown exception.";
		LogManager::getInstance()->message("[Update] Error: " + content);
	} else {
		try {
			SimpleXML xml;
			xml.fromXML(content);

			if(xml.findChild("ProfileVersionInfo")) {
				xml.stepIn();
				if(xml.findChild("Info")) {
					xml.stepIn();
					if(xml.findChild("Name")) {
						profilesCache.profileName = xml.getChildData();
					}
					if(xml.findChild("URL")) {
						profilesCache.profileUrl = xml.getChildData();
					}
					xml.stepOut();
					xml.resetCurrentChild();
				}
				if(xml.findChild("ClientProfile")) {
					xml.stepIn();
					if(xml.findChild("Version")) {
						profilesCache.clientProfile.first = xml.getChildData();
					}
					while(xml.findChild("Comment")) {
						profilesCache.clientProfile.second += xml.getChildData();
					}
					xml.stepOut();
					xml.resetCurrentChild();
				}
				if(xml.findChild("UserInfoProfiles")) {
					xml.stepIn();
					if(xml.findChild("Version")) {
						profilesCache.myInfoProfile.first = xml.getChildData();
					}
					while(xml.findChild("Comment")) {
						profilesCache.myInfoProfile.second += xml.getChildData();
					}
					xml.stepOut();
					xml.resetCurrentChild();
				}
				if(xml.findChild("IpWatch")) {
					xml.stepIn();
					if(xml.findChild("Version")) {
						profilesCache.ipWatchProfile.first = xml.getChildData();
					}
					while(xml.findChild("Comment")) {
						profilesCache.ipWatchProfile.second += xml.getChildData();
					}
					xml.stepOut();
					xml.resetCurrentChild();
				}
				/*if(xml.findChild("Autosearch")) {
					xml.stepIn();
					string ver, comment;
					if(xml.findChild("Version")) {
						ver = xml.getChildData();
					}
					if(xml.findChild("Comment")) {
						comment = xml.getChildData();
					}
					xml.stepOut();
					xml.resetCurrentChild();
				}
				if(xml.findChild("AdlSearch")) {
					xml.stepIn();
					string ver, comment;
					if(xml.findChild("Version")) {
						ver = xml.getChildData();
					}
					if(xml.findChild("Comment")) {
						comment = xml.getChildData();
					}
					xml.stepOut();
					xml.resetCurrentChild();
				}*/
				xml.stepOut();
			}
		} catch(const Exception& e) {
			LogManager::getInstance()->message("[Update] XML Exception: " + e.getError());
		}
	}
	// call listeners
	fire(UpdateManagerListener::VersionUpdated(), clientCache, profilesCache);
}

void UpdateManager::on(TimerManagerListener::Minute, uint64_t) throw() {
	if(minutes < 0) {
		Lock l(cs);
		runUpdate();
		minutes = Util::rand(180, 360);
		return;
	}
	minutes--;
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */