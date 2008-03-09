/*
 * Copyright (C) 2007 adrian_007, adrian-007 on o2 point pl
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
#include "Plugin.h"
#include "resource.h"
#include "PluginAPI.h"
#include "version.h"

#pragma warning(disable:4100)

Plugin::Plugin() {
	//set some info about plugin... name, version and icon id (if you dont' want icon, set -1)
	PluginAPI::setPluginInfo(PLUGIN_ID, "TestAPI v2", "2.00", IDB_BITMAP);
	//get a handle to window
	PluginAPI::getMainWnd(r_hwnd);

	PluginAPI::logMessage("*** TestAPI v2 loaded");
	PluginAPI::setSetting(PLUGIN_ID, "setting123", "Test of RSX++ PluginAPI");
}

Plugin::~Plugin() {
	PluginAPI::logMessage("*** TestAPI v2 unloaded");
}

bool Plugin::onIncommingMessage(iClient* c, const char* msg) {
	return false;
}

bool Plugin::onOutgoingMessage(iClient* c, const char* aMsg) {
	if(aMsg[0] == '/') {
		string msg(aMsg);
		if(msg == "/testapi") {
			c->iAddHubLine("*** TestAPI Plugin Message");
			return true;
		} else if(msg == "/testapi send") {
			c->iHubMessage("TestAPI chat message");
			return true;
		} else if(msg.substr(0, 5) == "/spm ") {
			string tmp1 = msg.substr(5);
			string::size_type i = tmp1.find(" ");
			if(i != string::npos) {
				string nick = tmp1.substr(0, i);
				iUser* iu = c->getUserByNick(nick.c_str());
				if(iu != NULL) {
					string m = tmp1.substr(i+1);
					iu->sendPM(m.c_str());
				}
				const string& logm = "sent to " + nick;
				PluginAPI::logMessage(logm.c_str());
			}
			return true;
		}
	}
	return false;
}

bool Plugin::onIncommingPM(iUser* from, const char* msg) {
	const string& logm = "private message from " + string(from->iGetNick()) + " hub: " + string(from->getUserClient()->iGetHubUrl());
	PluginAPI::logMessage(logm.c_str());
	return false;
}

bool Plugin::onOutgoingPM(iUser* to, const char* msg) {
	const string& logm = "private message to " + string(to->iGetNick()) + " hub: " + string(to->getUserClient()->iGetHubUrl());
	PluginAPI::logMessage(logm.c_str());
	return false;
}

void Plugin::onToolbarClick() {
	const string& tmp = PluginAPI::getSetting(PLUGIN_ID, "setting123");
	MessageBox(r_hwnd, PluginAPI::toW(tmp.c_str()), _T("iPlugin"), 0);
	PluginAPI::showToolTip("TestAPI Popup", "This is a test of RSX++ PluginAPI ;)", PluginAPI::TT_INFO);
}

void Plugin::onSettingsLoaded() {

}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */