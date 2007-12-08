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

bool Plugin::onIncommingMessage(Client* /*c*/, const string& /*msg*/) {
	return false;
}

bool Plugin::onOutgoingMessage(Client* c, const string& msg) {
	if(msg.compare("/plugin") == 0) {
		const wstring& tmp = _T("Test of RSX++ PluginAPI");
		PluginAPI::addHubLine(c, PluginAPI::fromW(tmp), PluginAPI::STYLE_GENERAL);
		return true;
	} else if(msg.compare("/psend") == 0) {
		PluginAPI::sendHubMessage(c, "I'm a message from plugin! :>");
		return true;
	}
	return false;
}

void Plugin::onToolbarClick() {
	const string& tmp = PluginAPI::getSetting(PLUGIN_ID, "setting123");
	MessageBox(r_hwnd, PluginAPI::toW(tmp).c_str(), _T("iPlugin"), 0);
	PluginAPI::showToolTip("TestAPI Popup", "This is a test of RSX++ PluginAPI ;)", PluginAPI::TT_INFO);
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */