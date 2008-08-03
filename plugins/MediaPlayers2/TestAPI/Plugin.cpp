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

#include <ClientInterface.h>
#include <UserInterface.h>

#include "Plugin.h"
#include "resource.h"
#include "version.h"
#include "MainDlg.h"

#include "../MediaPlayersLib/src/MediaPlayersLib.h"

Plugin::Plugin() {
	//get a handle to window
	PluginAPI::getMainWnd(r_hwnd);

	PluginAPI::logMessage("*** MediaPlayers Plugin loaded");
	PluginAPI::setSetting(PLUGIN_ID, "winamp", "/me %[title] (%[elapsed]/%[length])");
	PluginAPI::setSetting(PLUGIN_ID, "wmp", "/me %[title] (%[elapsed]/%[length])");
	PluginAPI::setSetting(PLUGIN_ID, "itunes", "/me %[title] (%[elapsed]/%[length])");
	PluginAPI::setSetting(PLUGIN_ID, "mpc", "/me %[title] (%[elapsed]/%[length])");
}

Plugin::~Plugin() {
	PluginAPI::logMessage("*** MediaPlayers Plugin unloaded");
}

dcpp::rString Plugin::parseCommand(const std::string& msg, bool& failed, bool& thirdPerson) const {
	if(msg == "/winamp" || msg == "/w") {
		string ret = MediaPlayersLib::getWinampSpam(PluginAPI::getSetting(PLUGIN_ID, "winamp").c_str());
		if(ret.empty()) {
			failed = true;
			return "Supported version of Winamp is not running";
		}
		thirdPerson = ret.compare(0, 4, "/me ") == 0;
		if(thirdPerson)
			ret = ret.substr(4);
		return ret.c_str();
	} else if(msg == "/wmp") {
		string ret = MediaPlayersLib::getWindowsMediaPlayerSpam(PluginAPI::getSetting(PLUGIN_ID, "wmp").c_str(), r_hwnd);
		if(ret.empty()) {
			failed = true;
			return "Supported version of Windows Media Player is not running";
		}
		thirdPerson = ret.compare(0, 4, "/me ") == 0;
		if(thirdPerson)
			ret = ret.substr(4);
		return ret.c_str();
	} else if(msg == "/itunes") {
		string ret = MediaPlayersLib::get_iTunesSpam(PluginAPI::getSetting(PLUGIN_ID, "itunes").c_str());
		if(ret.empty()) {
			failed = true;
			return "Supported version of iTunes is not running";
		}
		thirdPerson = ret.compare(0, 4, "/me ") == 0;
		if(thirdPerson)
			ret = ret.substr(4);
		return ret.c_str();
	} else if(msg == "/mpc") {
		string ret = MediaPlayersLib::getMediaPlayerClassicSpam(PluginAPI::getSetting(PLUGIN_ID, "mpc").c_str());
		if(ret.empty()) {
			failed = true;
			return "Supported version of Media Player Classic is not running";
		}
		thirdPerson = ret.compare(0, 4, "/me ") == 0;
		if(thirdPerson)
			ret = ret.substr(4);
		return ret.c_str();
	} else {
		failed = true;
		return rString(0);
	}
}

bool Plugin::onOutgoingMessage(dcpp::iClient* c, const dcpp::rString& aMsg) {
	if(aMsg[0] == '/') {
		string msg(aMsg);
		bool failed = false;
		bool thirdPerson = false;
		const rString ret = parseCommand(msg, failed, thirdPerson);
		if(!ret.empty()) {
			if(!failed)
				c->p_hubMessage(ret, thirdPerson);
			else
				c->p_addHubLine(ret, PluginAPI::STYLE_STATUS);
			return true;
		}
	}
	return false;
}

bool Plugin::onOutgoingPM(dcpp::iOnlineUser* to, const dcpp::rString& aMsg) {
	if(aMsg[0] == '/') {
		string msg(aMsg);
		bool failed = false;
		bool thirdPerson = false;
		const rString ret = parseCommand(msg, failed, thirdPerson);
		if(!ret.empty()) {
			if(!failed)
				to->p_sendPM(ret, thirdPerson);
			//else
			//{ }
			return true;
		}
	}
	return false;
}

void Plugin::onMainWndEvent(int type, HWND hWnd) {
	switch(type) {
	case 1: { // settings opened

#define SET_TEXT(id, name) ::SetWindowText(::GetDlgItem(hWnd, id), PluginAPI::fromUtf8ToWide(PluginAPI::getSetting(PLUGIN_ID, name)).c_str());
		SET_TEXT(IDC_WINAMP, "winamp");
		SET_TEXT(IDC_WMP, "wmp");
		SET_TEXT(IDC_ITUNES, "itunes");
		SET_TEXT(IDC_MPC, "mpc");
#undef SET_TEXT
		break;
			}
	case 2: { // settings close
#define GET_TEXT(id, name) len = ::GetWindowTextLength(::GetDlgItem(hWnd, id)) + 1;\
	buf.resize(len);\
	::GetWindowText(::GetDlgItem(hWnd, id), &buf[0], len);\
	PluginAPI::setSetting(PLUGIN_ID, name, PluginAPI::fromWideToUtf8(buf.c_str()));

		wstring buf;
		int len = 0;

		GET_TEXT(IDC_WINAMP, "winamp");
		GET_TEXT(IDC_WMP, "wmp");
		GET_TEXT(IDC_ITUNES, "itunes");
		GET_TEXT(IDC_MPC, "mpc");
#undef GET_TEXT
		break;
			}
	}
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */