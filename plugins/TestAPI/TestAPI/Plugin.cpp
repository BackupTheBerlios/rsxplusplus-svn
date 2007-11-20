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
#include "pluginDefs.h"

#include "Plugin.h"
#include "resource.h"

Plugin::Plugin() : hInstance(NULL) { /*at this point iPluginAPI == NULL! */ }
Plugin::~Plugin() { }

void Plugin::onLoad() {
	getAPI()->logMessage(_T("*** TestAPI.dll loaded"));
}

void Plugin::onUnload() {
	getAPI()->logMessage(_T("*** TestAPI.dll unloaded"));
}

void Plugin::setDefaults() {
	setSetting(_T("Setting"), _T("this text is stored in PluginSettings.xml ;)"));
}

HBITMAP Plugin::getPluginIcon() {
	//return NULL if you don't want to be in plugins toolbar ;)
	return (HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP), IMAGE_BITMAP, 12, 12, LR_SHARED);
}

void Plugin::onToolbarClick() {
	wchar_t buf[16];
	snwprintf(buf, sizeof(buf), L"Client SVN Revision: %d", getAPI()->getSVNRevision());

	std::wstring tmp = buf;
	tmp += _T("\r\nClient profiles version: ") + getAPI()->getClientProfileVersion();
	tmp += _T("\r\nMyINFO profiles version: ") + getAPI()->getMyInfoProfileVersion();
	tmp += _T("\r\n\r\nSetting str: ") + getSetting(_T("Setting"));
	MessageBox(getAPI()->getMainWnd(), tmp.c_str(), _T(PLUGIN_NAME), 0);
}

bool Plugin::onHubEnter(Client* client, const wstring& aMessage) {
	if(aMessage == _T("/plugin")) {
		getAPI()->addHubLine(client, _T("*** Hello world! I'm working on ") + getAPI()->getHubName(client), 3);
		return true;
	} else if(aMessage == _T("/help")) {
		getAPI()->addHubLine(client, _T("*** some help info from plugin"), 3);
		return true;
	}
	return false;
}

bool Plugin::onHubMessage(Client* /*client*/, const wstring& /*aMessage*/) {
	//raw data from hub
	return false;
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */