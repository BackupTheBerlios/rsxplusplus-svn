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

#include "stdinc.h"

#include <ClientInterface.h>
#include <UserInterface.h>

#include "Plugin.h"
#include "resource.h"
#include "version.h"

Plugin::Plugin() {
	// get a handle to window
	PluginAPI::getMainWnd(r_hwnd);

	PluginAPI::logMessage("*** TestAPI v2 loaded");
	PluginAPI::setSetting(PLUGIN_ID, "testapi_setting", "variable");
	PluginAPI::registerHubSetting("TEST", "default value", "Test Setting");
}

Plugin::~Plugin() {
	PluginAPI::logMessage("*** TestAPI v2 unloaded");
}

bool Plugin::onOutgoingMessage(iClient* c, const rString& aMsg) {
	if(!aMsg.empty() && aMsg[0] == '/') {
		string msg(aMsg);
		if(msg == "/testapi") {
			c->p_addHubLine("Test Message in StatusBar :)", PluginAPI::STYLE_STATUS);
			string mynick = c->p_getMyField("NI").c_str();
			string message = "*** User " + mynick + " - too many same numbers in share....";
			c->p_addHubLine(message.c_str(), PluginAPI::STYLE_CHEAT);
			return true;
		} else if(msg == "/testapi send") {
			c->p_hubMessage("RSX++ PluginAPI Test Message");
			return true;
		}
	}
	return false;
}

void Plugin::onMainWndEvent(int type, HWND hWnd) {
	switch(type) {
	case 0: {
		rString setting = PluginAPI::getSetting(PLUGIN_ID, "testapi_setting");
		MessageBox(r_hwnd, PluginAPI::fromUtf8ToWide(setting).c_str(), L"TestAPI::onToolBarClick()", MB_ICONINFORMATION);
		break;
			}
	case 1: { // settings opened
		const rString& str = PluginAPI::getSetting(PLUGIN_ID, "testapi_setting");
		::SetWindowText(::GetDlgItem(hWnd, IDC_EDIT1), PluginAPI::fromUtf8ToWide(str).c_str());
		break;
			}
	case 2: { // settings close
		wstring tmp;
		int len = ::GetWindowTextLength(GetDlgItem(hWnd, IDC_EDIT1)) + 1;
		tmp.resize(len);
		::GetWindowText(::GetDlgItem(hWnd, IDC_EDIT1), &tmp[0], len);
		PluginAPI::setSetting(PLUGIN_ID, "testapi_setting", PluginAPI::fromWideToUtf8(tmp.c_str()));
		break;
			}
	}
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */