/*
 * Copyright (C) 2007-2009 adrian_007, adrian-007 on o2 point pl
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

	ctrlFavGroups.Attach(GetDlgItem(IDC_RSX_FAV_GROUPS));
	ctrlFavGroups.GetClientRect(rc);
	ctrlFavGroups.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, (rc.Width() - 17), 0);
	ctrlFavGroups.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT/* | LVS_EX_INFOTIP*/);

	const StringList& lst = FavoriteManager::getInstance()->getFavGroups();
	for(StringList::const_iterator j = lst.begin(); j != lst.end(); ++j)
		ctrlFavGroups.insert(ctrlFavGroups.GetItemCount(), Text::toT((*j)));

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

	StringList& lst = FavoriteManager::getInstance()->getFavGroups();
	if(!lst.size()) FavoriteManager::getInstance()->addFavGroup("All Hubs");
	FavoriteManager::getInstance()->save();
	
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

LRESULT RSXPage::onFavGroupBtn(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL&  bHandled) {
	switch(wID) {
		case IDC_RSX_FAV_ADD: {
			LineDlg dlg;
			dlg.title = _T("Add Favorite Hub Group");
			while(true) {
				if(dlg.DoModal() == IDOK) {
					if(!FavoriteManager::getInstance()->addFavGroup(Text::fromT(dlg.line))) {
						MessageBox(_T("This group already exist!"));
					} else {
						ctrlFavGroups.insert(ctrlFavGroups.GetItemCount(), dlg.line);
						break;
					}						
				} else {
					break;
				}
			}
			return 0;
		}
		case IDC_RSX_FAV_EDIT: {
			if(ctrlFavGroups.GetSelectedCount() == 1) {
				int sel = ctrlFavGroups.GetSelectedIndex();
				TCHAR buf[256];
				ctrlFavGroups.GetItemText(sel, 0, buf, 256);

				LineDlg dlg;
				dlg.title = _T("Edit Favorite Hub Group");
				dlg.line = buf;
				while(true) {
					if(dlg.DoModal() == IDOK) {
						if(!FavoriteManager::getInstance()->editFavGroup((uint8_t)sel, Text::fromT(dlg.line))) {
							MessageBox(_T("This group already exist!"));
						} else {
							ctrlFavGroups.SetItemText(sel, 0, (dlg.line).c_str());
							break;
						}						
					} else {
						break;
					}
				}
			}
			return 0;
		}
		case IDC_RSX_FAV_REMOVE: {
			if(ctrlFavGroups.GetSelectedCount() == 1) {
				int pos = ctrlFavGroups.GetSelectedIndex();
				FavoriteManager::getInstance()->removeFavGroup((uint8_t)pos);
				ctrlFavGroups.DeleteItem(pos);
			}
			return 0;
		}
		default: break;
	}
	bHandled = FALSE;
	return 0;
}
