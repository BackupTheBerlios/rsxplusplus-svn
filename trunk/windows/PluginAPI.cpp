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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "../client/PluginAPI/PluginAPI.h"
#include "../client/PluginsManager.h"

#include "../client/Text.h"
#include "../client/LogManager.h"
#include "../client/ClientProfileManager.h"
#include "../client/DetectionManager.h"

#include "../rsx/Wildcards.h"
#include "../rsx/RegexUtil.h"
#include "../rsx/IpManager.h"
#include "../rsx/AutoSearchManager.h"

#include "MainFrm.h"
#include "HubFrame.h"

namespace dcpp {

rString::rString(const char* str) {
	int len = strlen(str);
	buf = new char[len+1];
	if(!buf) return;
	strcpy(buf, str);
}

rString::~rString() {
	if(buf) {
		delete buf;
		buf = 0;
	}
}

bool rString::empty() const {
	if(buf == NULL)
		return true;
	if(buf == '\0')
		return true;
	return false;
}

void PluginAPI::logMessage(const rString& aMsg) {
	LogManager::getInstance()->message(aMsg.c_str());
}

rString PluginAPI::getSetting(int id, const rString& aName) {
	return PluginsManager::getInstance()->getSetting(id, string(aName)).c_str();
}

void PluginAPI::setSetting(int id, const rString& aName, const rString& aVal) {
	PluginsManager::getInstance()->setSetting(id, aName.c_str(), aVal.c_str());
}

void PluginAPI::getMainWnd(HWND& h) {
	h = MainFrame::getMainFrame()->m_hWnd;
}

void PluginAPI::showToolTip(const rString& pTitle, const rString& pMsg, int pIcon) {
	MainFrame::getMainFrame()->ShowBalloonTip(Text::toT(pMsg.c_str()), Text::toT(pTitle.c_str()), pIcon);
}

rString PluginAPI::getVersion(int type) {
	switch(type) {
		case CLIENT_PROFILE:
			return ClientProfileManager::getInstance()->getProfileVersion().c_str();
		case MYINFO_PROFILE:
			return ClientProfileManager::getInstance()->getMyinfoProfileVersion().c_str();
		case IP_WATCH_PROFILE:
			return IpManager::getInstance()->getIpWatchVersion().c_str();
		case AUTOSEARCH_PROFILE:
			return AutoSearchManager::getInstance()->getVersion().c_str();
		case ADLS_PROFILE:
			return "1.00";
		case RSX_VERSION:
			return VERSIONSTRING;
		case RSX_REVISION:
			return "0";
		default:
			return "0.00";
	}
}

rString PluginAPI::getDataPath() {
	return Util::getDataPath().c_str();
}

rString PluginAPI::getClientSetting(const rString& aName, bool rsxmng) {
	if(rsxmng) {
		return RSXSettingsManager::getInstance()->getString(aName.c_str()).c_str();
	} else {
		return SettingsManager::getInstance()->getString(aName.c_str()).c_str();
	}
}

int PluginAPI::getClientSettingInt(const rString& aName, bool rsxmng) {
	if(rsxmng) {
		return RSXSettingsManager::getInstance()->getInt(aName.c_str());
	} else {
		return SettingsManager::getInstance()->getInt(aName.c_str());
	}
}

bool PluginAPI::RegExMatch(const rString& strToMatch, const rString& regEx, const char* /*opt*/) {
	return RegexUtil::match(strToMatch.c_str(), regEx.c_str());
}

bool PluginAPI::WildcardMatch(const rString& strToMatch, const rString& pattern, char delim, bool useSet) {
	return Wildcard::patternMatch(strToMatch.c_str(), pattern.c_str(), delim, useSet);
}

void PluginAPI::OpenHub(const rString& aHubUrl) {
	HubFrame::openWindow(Text::toT(aHubUrl.c_str()));
}

void PluginAPI::CloseHub(const rString& aHubUrl) {
	HubFrame::closeHub(Text::toT(aHubUrl.c_str()));
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */