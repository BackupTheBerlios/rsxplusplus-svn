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
#include "../client/DetectionManager.h"
#include "../client/ConnectionManager.h"
#include "../client/AdcHub.h"
#include "../client/NmdcHub.h"

#include "../rsx/Wildcards.h"
#include "../rsx/RegexUtil.h"
#include "../rsx/IpManager.h"
#include "../rsx/AutoSearchManager.h"

#include "MainFrm.h"
#include "HubFrame.h"

namespace dcpp {
namespace rStrUtil {
	inline size_t strlen(const char* c) { return std::strlen(c); }
	inline size_t strlen(const wchar_t* c) { return std::wcslen(c); }
	inline char* strcpy(char* dest, const char* src) { return std::strcpy(dest, src); }
	inline wchar_t* strcpy(wchar_t* dest, const wchar_t* src) { return wcscpy(dest, src); }
	// dummy functions, just in case...
	template<class T> size_t strlen(const T*) { return 0; }
	template<class T> T* strcpy(T*, const T*) { return NULL; }
}

template<class T>
rStringBase<T>::rStringBase(const T* str) {
	if(str == 0) { buf = 0; return; }

	int len = rStrUtil::strlen(str);
	buf = new T[len+1];
	if(!buf) return;
	rStrUtil::strcpy(buf, str);
}

template<class T>
rStringBase<T>::~rStringBase() {
	if(buf) {
		delete buf;
		buf = 0;
	}
}

// basic string conversions
rString PluginAPI::AcpToUtf8(const rString& str) {			return Text::acpToUtf8(str.c_str()).c_str(); }
rString PluginAPI::Utf8ToAcp(const rString& str) {			return Text::utf8ToAcp(str.c_str()).c_str(); }
rString PluginAPI::fromWideToUtf8(const rStringW& str) {	return Text::wideToUtf8(str.c_str()).c_str(); }
rString PluginAPI::fromWideToAcp(const rStringW& str) {		return Text::wideToAcp(str.c_str()).c_str(); }
rStringW PluginAPI::fromUtf8ToWide(const rString& str) {	return Text::utf8ToWide(str.c_str()).c_str(); }
rStringW PluginAPI::fromAcpToWide(const rString& str) {		return Text::acpToWide(str.c_str()).c_str(); }

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
		case CLIENT_PROFILE:		return DetectionManager::getInstance()->getProfileVersion().c_str();
		case USER_INFO_PROFILE:		return DetectionManager::getInstance()->getUserInfoVersion().c_str();
		case IP_WATCH_PROFILE:		return IpManager::getInstance()->getIpWatchVersion().c_str();
		case AUTOSEARCH_PROFILE:	return AutoSearchManager::getInstance()->getVersion().c_str();
		case RSX_VERSION:			return VERSIONSTRING;
		case RSX_REVISION:			return BOOST_STRINGIZE(SVN_REVISION);
		default:					return "";
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

void PluginAPI::registerHubSetting(const char* name, const rString& aValue, const rString& description) {
	FavoriteManager::registerHubSetting(name, aValue.c_str(), description.c_str());
}

bool PluginAPI::RegexMatch(const rString& strToMatch, const rString& regEx) {
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

void PluginAPI::AddConnectionFeature(const rString& feat, bool isAdc) {
	ConnectionManager::getInstance()->addFeature(feat.c_str(), isAdc);
}

void PluginAPI::RemoveConnectionFeature(const rString& feat, bool isAdc) {
	ConnectionManager::getInstance()->remFeature(feat.c_str(), isAdc);
}

void PluginAPI::AddHubFeature(const rString& feat, bool isAdc) {
	if(isAdc) {
		AdcHub::addFeature(feat.c_str());
	} else {
		NmdcHub::addFeature(feat.c_str());
	}
}

} // namespace dcpp

/**
 * @file
 * $Id$
 */