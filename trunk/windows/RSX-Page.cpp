/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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
#include "../client/FavoriteManager.h"
#include "../client/version.h"
#include "../client/rsxppSettingsManager.h"

#include "RSX-Page.h"
#include "LineDlg.h"
#include "SpellChecker.hpp"

PropPage::TextItem RSXPage::texts[] = {
	{ IDC_RSX_FAV_ADD,			ResourceManager::ADD },
	{ IDC_RSX_FAV_EDIT,			ResourceManager::EDIT_ACCEL },
	{ IDC_RSX_FAV_REMOVE,		ResourceManager::REMOVE },
	{ IDC_STARTUP_PRIO_TEXT,	ResourceManager::SETTINGS_STARTUP_PRIORITY },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item RSXPage::items[] = {
	{ IDC_CHAT_BUF_SIZE,		rsxppSettingsManager::MAX_CHAT_BUFSIZE,			PropPage::T_INT_RSX },
	{ 0, 0, PropPage::T_END }
};

RSXPage::ListItem RSXPage::listItems[] = {
	{ rsxppSettingsManager::AUTO_START, ResourceManager::SETTINGS_AUTO_START },
	{ rsxppSettingsManager::USE_FILTER_FAV, ResourceManager::USE_FILTER_FAV },
	{ rsxppSettingsManager::USE_HL_FAV, ResourceManager::USE_HL_FAV },
	{ rsxppSettingsManager::FLASH_WINDOW_ON_PM, ResourceManager::FLASH_WINDOW_ON_PM },
	{ rsxppSettingsManager::FLASH_WINDOW_ON_NEW_PM, ResourceManager::FLASH_WINDOW_ON_NEW_PM },
	{ rsxppSettingsManager::IP_IN_CHAT, ResourceManager::IP_IN_CHAT },
	{ rsxppSettingsManager::COUNTRY_IN_CHAT, ResourceManager::COUNTRY_IN_CHAT },
	{ rsxppSettingsManager::SHOW_LUA_ERROR_MESSAGE, ResourceManager::SETTINGS_SHOW_LUA_ERROR_MESSAGE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT RSXPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_RSX_BOOLEANS), true);

	CRect rc;

	ctrlPrio.Attach(GetDlgItem(IDC_STARTUP_PRIO));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_REALTIME));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_HIGH));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_ABOVE));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_NORMAL));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_BELOW));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_IDLE));
	ctrlPrio.SetCurSel(RSXPP_SETTING(DEFAULT_PRIO));

	ctrlDictionary.Attach(GetDlgItem(IDC_DICTIONARY));
	ctrlDictionary.AddString(_T("None/Disabled"));

	WIN32_FIND_DATA data;
	HANDLE hFind;
	DWORD attr;
	tstring cfg(Text::toT(Util::getPath(Util::PATH_USER_CONFIG)));

	hFind = FindFirstFile(Text::toT(Util::getPath(Util::PATH_USER_CONFIG) + "*.dic").c_str(), &data);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			tstring name = data.cFileName;
			tstring::size_type i = name.rfind('.');
			name = name.substr(0, i);

			attr = GetFileAttributes(tstring(cfg + name + _T(".aff")).c_str());
			if(attr != 0xFFFFFFFF) {
				ctrlDictionary.AddString(name.c_str());
			}
		} while(FindNextFile(hFind, &data));
		FindClose(hFind);
	}

	int pos = ctrlDictionary.FindString(0, Text::toT(RSXPP_SETTING(DICTIONARY)).c_str());
	if(pos < 0) pos = 0;
	ctrlDictionary.SetCurSel(pos);

	return TRUE;
}

void RSXPage::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_RSX_BOOLEANS), true);
	rsxppSettingsManager::getInstance()->set(rsxppSettingsManager::DEFAULT_PRIO, ctrlPrio.GetCurSel());
	
	HKEY hk;
	tstring app = _T("\"") + Text::toT(Util::getPath(Util::PATH_GLOBAL_CONFIG)) + _T("RSXPlusPlus.exe\"");
	if(::RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_WRITE | KEY_READ, &hk) == ERROR_SUCCESS) {
		if(RSXPP_BOOLSETTING(AUTO_START)) {
			::RegCreateKey(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hk);
			::RegSetValueEx(hk, _T(APPNAME), 0, REG_SZ, (LPBYTE)app.c_str(), sizeof(TCHAR) * (app.length() + 1));
		} else {
			::RegDeleteValue(hk, _T(APPNAME));
		}
		::RegCloseKey(hk);
	}
	if(ctrlDictionary.GetCurSel() > 0) {
		tstring buf;
		buf.resize(ctrlDictionary.GetLBTextLen(ctrlDictionary.GetCurSel()));
		ctrlDictionary.GetLBText(ctrlDictionary.GetCurSel(), &buf[0]);
		RSXPP_SET(DICTIONARY, Text::fromT(buf));
	} else {
		RSXPP_SET(DICTIONARY, Util::emptyString);
	}
}
