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
#include "../client/DCPlusPlus.h"
#include "../client/SettingsManager.h"
#include "Resource.h"

#include "RSX-Advanced.h"
#include "CommandDlg.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"

#include "WinUtil.h"
#define ATTACH(id, var) var.Attach(GetDlgItem(id))
#define attach_spin(id, var ,min, max) var.Attach(GetDlgItem(id)); \
	var.SetRange32(min, max); \
	var.Detach();

PropPage::TextItem RSXAdvanced::texts[] = {
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item RSXAdvanced::items[] = {
	{ IDC_MYINFO_COUNTER,			RSXSettingsManager::MYINFO_COUNTER,			PropPage::T_INT_RSX },
	{ IDC_CTE_COUNTER,				RSXSettingsManager::CTM_COUNTER,			PropPage::T_INT_RSX },
	{ IDC_PM_COUNTER,				RSXSettingsManager::PM_COUNTER,				PropPage::T_INT_RSX },
	{ IDC_MYINFO_TIME,				RSXSettingsManager::MYINFO_TIME,			PropPage::T_INT_RSX },
	{ IDC_CTE_TIME,					RSXSettingsManager::CTM_TIME,				PropPage::T_INT_RSX },
	{ IDC_PM_TIME,					RSXSettingsManager::PM_TIME,				PropPage::T_INT_RSX },
	{ IDC_RAW_SENDER_SLEEP_TIME,	RSXSettingsManager::RAW_SENDER_SLEEP_TIME,	PropPage::T_INT_RSX },
	{ 0, 0, PropPage::T_END }
};

RSXAdvanced::ListItem RSXAdvanced::listItems[] = {
	{ RSXSettingsManager::FAV_USER_IS_PROTECTED_USER,			ResourceManager::SETTINGS_FAV_USER_IS_PROTECTED_USER },
	{ RSXSettingsManager::USE_SEND_DELAYED_RAW,					ResourceManager::SETTINGS_USE_SEND_DELAYED_RAW },
	{ RSXSettingsManager::DISPLAY_CHEATS_IN_MAIN_CHAT,			ResourceManager::SETTINGS_DISPLAY_CHEATS_IN_MAIN_CHAT },
	{ RSXSettingsManager::SHOW_SHARE_CHECKED_USERS,				ResourceManager::SETTINGS_ADVANCED_SHOW_SHARE_CHECKED_USERS },
	{ RSXSettingsManager::UNCHECK_CLIENT_PROTECTED_USER,		ResourceManager::SETTINGS_UNCHECK_CLIENT_PROTECTED_USER },
	{ RSXSettingsManager::UNCHECK_LIST_PROTECTED_USER,			ResourceManager::SETTINGS_UNCHECK_LIST_PROTECTED_USER },
	{ RSXSettingsManager::CHECK_ALL_CLIENTS_BEFORE_FILELISTS,	ResourceManager::SETTINGS_CHECK_ALL_CLIENTS_BEFORE_FILELISTS },
	{ RSXSettingsManager::DELETE_CHECKED_FILELISTS, 			ResourceManager::SETTINGS_DELETE_CHECKED_FILELISTS },
	{ RSXSettingsManager::IGNORE_PM_SPAMMERS,					ResourceManager::SETTINGS_IGNORE_PM_SPAMMERS },
	{ RSXSettingsManager::PROTECT_PM_USERS,						ResourceManager::SETTINGS_PROTECT_PM_USERS },
	{ RSXSettingsManager::USE_WILDCARDS_TO_PROTECT,				ResourceManager::SETTINGS_USE_WILDCARDS_TO_PROTECT },
	{ RSXSettingsManager::GET_UPDATE_PROFILE,					ResourceManager::SETTINGS_GET_UPDATE_PROFILE },
	{ RSXSettingsManager::SHOW_CLIENT_NEW_VER,					ResourceManager::SETTINGS_SHOW_CLIENT_NEW_VER },
	{ RSXSettingsManager::SHOW_MYINFO_NEW_VER,					ResourceManager::SETTINGS_SHOW_MYINFO_NEW_VER },
	{ RSXSettingsManager::SHOW_IPWATCH_NEW_VER,					ResourceManager::SETTINGS_SHOW_IPWATCH_NEW_VER },

	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT RSXAdvanced::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_RSXADVANCED_BOOLEANS), true); //RSX++ // list items

	CUpDownCtrl spin;
	attach_spin(IDC_MYINFO_COUNTER_SPIN,			spin, 0, 99);
	attach_spin(IDC_CTE_COUNTER_SPIN,				spin, 0, 99);
	attach_spin(IDC_PM_COUNTER_SPIN,				spin, 0, 99);
	attach_spin(IDC_MYINFO_TIME_SPIN,				spin, 200, 9000);
	attach_spin(IDC_CTE_TIME_SPIN,					spin, 200, 9000);
	attach_spin(IDC_PM_TIME_SPIN,					spin, 200, 9000);
	attach_spin(IDC_RAW_SENDER_SLEEP_TIME_SPIN,		spin, 0, 5000);
	return TRUE;
}

void RSXAdvanced::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_RSXADVANCED_BOOLEANS), true); //RSX++ // list items
}