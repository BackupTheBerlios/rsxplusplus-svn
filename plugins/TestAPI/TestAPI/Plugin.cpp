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

#include <ClientInterface.h>
#include <UserInterface.h>

#include "Plugin.h"
#include "resource.h"
#include "version.h"
#include <rsxppStringUtils.hpp>

Plugin::Plugin() {
	//get a handle to window
	PluginAPI::getMainWnd(r_hwnd);

	PluginAPI::logMessage("*** TestAPI v2 loaded");
	PluginAPI::setSetting(PLUGIN_ID, "setting123", "Test of RSX++ PluginAPI");
}

Plugin::~Plugin() {
	PluginAPI::logMessage("*** TestAPI v2 unloaded");
}

bool Plugin::onOutgoingMessage(dcpp::iClient* c, const rsxpp::String& aMsg) {
	if(aMsg[0] == '/') {
		string msg(aMsg);
		if(msg == "/testapi") {
			const String& msgToSend = StringUtils::Format("*** TestAPI\n\tHub Name: %s\n\tHub URL: %s", c->p_getField("NI").c_str(), c->p_getHubUrl().c_str());
			c->p_addHubLine(msgToSend);
			return true;
		} else if(msg == "/testapi send") {
			c->p_hubMessage("RSX++ PluginAPI Test Message");
			return true;
		} else if(msg.substr(0, 5) == "/spm ") {
			string tmp1 = msg.substr(5);
			string::size_type i = tmp1.find(" ");
			if(i != string::npos) {
				string nick = tmp1.substr(0, i);
				dcpp::iOnlineUser* iu = c->p_getUserByNick(nick.c_str());
				if(iu != NULL) {
					string m = tmp1.substr(i+1);
					iu->p_sendPM(m.c_str());
				}
			}
			return true;
		}
	}
	return false;
}

bool Plugin::onIncommingPM(dcpp::iOnlineUser* from, const rsxpp::String&) {
	const String& msgToSend = StringUtils::Format("Private Message from %s (Hub: %s)", from->p_getNick().c_str(), from->p_getUserClient()->p_getField("NI").c_str());
	PluginAPI::logMessage(msgToSend);
	return false;
}

bool Plugin::onOutgoingPM(dcpp::iOnlineUser* to, const rsxpp::String&) {
	const String& msgToSend = StringUtils::Format("Private Message to %s (Hub: %s)", to->p_getNick().c_str(), to->p_getUserClient()->p_getField("NI").c_str());
	PluginAPI::logMessage(msgToSend);
	return false;
}

void Plugin::onToolbarClick() {
	const String& setting = PluginAPI::getSetting(PLUGIN_ID, "setting123");
	//String str = StringUtils::Format("Setting Value: %s", setting.c_str());

	String str = setting;
	String::size_type i = 0;
	while((i = str.find("PluginAPI", i)) != String::npos) {
		str.replace(i, 9, "Software Development Kit");
		i++;
	}
	MessageBoxA(r_hwnd, str, "ANSI Dialog - TestAPI", 0);
}

/**
 * @file
 * $Id: Plugin.cpp 42 2007-10-31 18:27:40Z adrian_007 $
 */