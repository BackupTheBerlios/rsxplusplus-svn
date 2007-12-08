/* 
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

#include "../../client/Client.h"
#include "../../client/Text.h"
#include "../../client/LogManager.h"

#include "../../windows/MainFrm.h"

void PluginAPI::setPluginInfo(int id, const string n, const string v, int i) {
	PluginsManager::getInstance()->setPluginInfo(id, n, v, i);
}

string PluginAPI::fromW(const wstring& aStr) {
	return Text::fromT(aStr);
}

wstring PluginAPI::toW(const string& aStr) {
	return Text::toT(aStr);
}

void PluginAPI::logMessage(const string& aMsg) {
	LogManager::getInstance()->message(aMsg);
}

void PluginAPI::logMessage(const wstring& aMsg) {
	logMessage(Text::fromT(aMsg));
}

void PluginAPI::addHubLine(Client* client, const string& aMsg, int mType) {
	if(!client) return;
	Lock l(client->cs);
	client->addHubLine(aMsg, mType);
}

void PluginAPI::addHubLine(Client* client, const wstring& aMsg, int mType) {
	addHubLine(client, Text::fromT(aMsg), mType);
}

void PluginAPI::sendHubMessage(Client* client, const string& aMsg) {
	if(!client) return;
	Lock l(client->cs);
	client->hubMessage(aMsg);
}

void PluginAPI::sendHubMessage(Client* client, const wstring& aMsg) {
	sendHubMessage(client, Text::fromT(aMsg));
}

const string& PluginAPI::getSetting(int id, const string& aName) {
	return PluginsManager::getInstance()->getSetting(id, aName);
}

void PluginAPI::setSetting(int id, const string& aName, const string& aVal) {
	PluginsManager::getInstance()->setSetting(id, aName, aVal);
}

string PluginAPI::formatParams(const string& frm, std::tr1::unordered_map<string, string>& params) {
	return Util::formatParams(frm, params, false);
}

void PluginAPI::getMainWnd(HWND& h) {
	h = MainFrame::getMainFrame()->m_hWnd;
}

void PluginAPI::showToolTip(const string& pTitle, const string& pMsg, int pIcon) {
	showToolTip(Text::toT(pTitle), Text::toT(pMsg), pIcon);
}

void PluginAPI::showToolTip(const wstring& pTitle, const wstring& pMsg, int pIcon) {
	MainFrame::getMainFrame()->ShowBalloonTip(pMsg.c_str(), pTitle.c_str(), pIcon);
}

/**
 * @file
 * $Id: MainFrm.cpp,v 1.20 2004/07/21 13:15:15 bigmuscle Exp
 */