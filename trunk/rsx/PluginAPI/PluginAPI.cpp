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

#include "../../windows/stdafx.h"
#include "../../client/DCPlusPlus.h"

#include "PluginAPI.h"
#include "PluginsManager.h"

#include "../../client/Text.h"
#include "../../client/LogManager.h"
#include "../../client/ClientProfileManager.h"
#include "../Wildcards.h"
#include "../../client/pme.h"

#include "../IpManager.h"
#include "../AutoSearchManager.h"

#include "../../windows/MainFrm.h"

void PluginAPI::setPluginInfo(int id, const char* n, const char* v, int i) {
	PluginsManager::getInstance()->setPluginInfo(id, string(n), string(v), i);
}

const char* PluginAPI::fromW(const wchar_t* aStr) {
	return Text::fromT(wstring(aStr)).c_str();
}

const wchar_t* PluginAPI::toW(const char* aStr) {
	return Text::toT(string(aStr)).c_str();
}

void PluginAPI::logMessage(const char* aMsg) {
	LogManager::getInstance()->message(string(aMsg));
}

void PluginAPI::logMessage(const wchar_t* aMsg) {
	LogManager::getInstance()->message(Text::fromT(wstring(aMsg)));
}

const char* PluginAPI::getSetting(int id, const char* aName) {
	return PluginsManager::getInstance()->getSetting(id, string(aName)).c_str();
}

void PluginAPI::setSetting(int id, const char* aName, const char* aVal) {
	PluginsManager::getInstance()->setSetting(id, string(aName), string(aVal));
}

const char* PluginAPI::formatParams(const char* frm, std::vector<pair<char*, char*> >& params) {
	StringMap tmp;
	for(std::vector<pair<char*, char*> >::const_iterator i = params.begin(); i != params.end(); ++i)
		tmp.insert(make_pair((*i).first, (*i).second));

	string ret = Util::formatParams(string(frm), tmp, false);
	return ret.c_str();
}

void PluginAPI::getMainWnd(HWND& h) {
	h = MainFrame::getMainFrame()->m_hWnd;
}

void PluginAPI::showToolTip(const char* pTitle, const char* pMsg, int pIcon) {
	showToolTip(Text::toT(string(pTitle)).c_str(), Text::toT(string(pMsg)).c_str(), pIcon);
}

void PluginAPI::showToolTip(const wchar_t* pTitle, const wchar_t* pMsg, int pIcon) {
	MainFrame::getMainFrame()->ShowBalloonTip(pMsg, pTitle, pIcon);
}

const char* PluginAPI::getVersion(int type) {
	switch(type) {
		case CLIENT_PROFILE: return ClientProfileManager::getInstance()->getProfileVersion().c_str();
		case MYINFO_PROFILE: return ClientProfileManager::getInstance()->getMyinfoProfileVersion().c_str();
		case IP_WATCH_PROFILE: return IpManager::getInstance()->getIpWatchVersion().c_str();
		case AUTOSEARCH_PROFILE: return AutoSearchManager::getInstance()->getVersion().c_str();
		case ADLS_PROFILE: return "1.00";
		case RSX_VERSION: return VERSIONSTRING;
		case RSX_REVISION: return "0";
		default: return "0.00";
	}
}

const char* PluginAPI::getDataPath() {
	return Util::getDataPath().c_str();
}

const char* PluginAPI::getClientSetting(const char* aName, bool rsxmng/* = false*/) {
	if(rsxmng) {
		return RSXSettingsManager::getInstance()->getString(string(aName)).c_str();
	} else {
		return SettingsManager::getInstance()->getString(string(aName)).c_str();
	}
}

int PluginAPI::getClientSettingInt(const char* aName, bool rsxmng/* = false*/) {
	if(rsxmng) {
		return RSXSettingsManager::getInstance()->getInt(string(aName));
	} else {
		return SettingsManager::getInstance()->getInt(string(aName));
	}
}

bool PluginAPI::RegExMatch(const char* strToMatch, const char* regEx, const char* opt /*= ""*/) {
	PME reg(regEx, opt);
	if(reg.IsValid()) {
		return reg.match(strToMatch) > 0;
	}
	return false;
}

bool PluginAPI::WildcardMatch(const char* strToMatch, const char* pattern, char delim, bool useSet) {
	return Wildcard::patternMatch(strToMatch, pattern, delim, useSet);
}

/**
 * @file
 * $Id$
 */