/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#include "../client/rsxppSettingsManager.h"

#include "WinUtil.h"

#define attach_spin(id, var ,min, max) var.Attach(GetDlgItem(id)); \
	var.SetRange32(min, max); \
	var.Detach();

PropPage::TextItem RSXAdvanced::texts[] = {
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item RSXAdvanced::items[] = {
	{ IDC_MYINFO_COUNTER,			rsxppSettingsManager::MYINFO_COUNTER,			PropPage::T_INT_RSX },
	{ IDC_CTE_COUNTER,				rsxppSettingsManager::CTM_COUNTER,			PropPage::T_INT_RSX },
	{ IDC_PM_COUNTER,				rsxppSettingsManager::PM_COUNTER,				PropPage::T_INT_RSX },
	{ IDC_MYINFO_TIME,				rsxppSettingsManager::MYINFO_TIME,			PropPage::T_INT_RSX },
	{ IDC_CTE_TIME,					rsxppSettingsManager::CTM_TIME,				PropPage::T_INT_RSX },
	{ IDC_PM_TIME,					rsxppSettingsManager::PM_TIME,				PropPage::T_INT_RSX },
	{ IDC_PROFILE_VERSION_URL,		rsxppSettingsManager::PROFILE_VER_URL,		PropPage::T_STR_RSX },
	{ 0, 0, PropPage::T_END }
};

RSXAdvanced::ListItem RSXAdvanced::listItems[] = {
	{ rsxppSettingsManager::FAV_USER_IS_PROTECTED_USER,			ResourceManager::SETTINGS_FAV_USER_IS_PROTECTED_USER },
	{ rsxppSettingsManager::USE_SEND_DELAYED_RAW,				ResourceManager::SETTINGS_USE_SEND_DELAYED_RAW },
	{ rsxppSettingsManager::DISPLAY_CHEATS_IN_MAIN_CHAT,		ResourceManager::SETTINGS_DISPLAY_CHEATS_IN_MAIN_CHAT },
	{ rsxppSettingsManager::SHOW_SHARE_CHECKED_USERS,			ResourceManager::SETTINGS_ADVANCED_SHOW_SHARE_CHECKED_USERS },
	{ rsxppSettingsManager::UNCHECK_CLIENT_PROTECTED_USER,		ResourceManager::SETTINGS_UNCHECK_CLIENT_PROTECTED_USER },
	{ rsxppSettingsManager::UNCHECK_LIST_PROTECTED_USER,		ResourceManager::SETTINGS_UNCHECK_LIST_PROTECTED_USER },
	{ rsxppSettingsManager::CHECK_ALL_CLIENTS_BEFORE_FILELISTS,	ResourceManager::SETTINGS_CHECK_ALL_CLIENTS_BEFORE_FILELISTS },
	{ rsxppSettingsManager::DELETE_CHECKED_FILELISTS, 			ResourceManager::SETTINGS_DELETE_CHECKED_FILELISTS },
	{ rsxppSettingsManager::IGNORE_PM_SPAMMERS,					ResourceManager::SETTINGS_IGNORE_PM_SPAMMERS },
	{ rsxppSettingsManager::PROTECT_PM_USERS,					ResourceManager::SETTINGS_PROTECT_PM_USERS },
	{ rsxppSettingsManager::USE_WILDCARDS_TO_PROTECT,			ResourceManager::SETTINGS_USE_WILDCARDS_TO_PROTECT },
	{ rsxppSettingsManager::GET_UPDATE_PROFILE,					ResourceManager::SETTINGS_GET_UPDATE_PROFILE },
	{ rsxppSettingsManager::UPDATE_PROFILE_CHEATS,				ResourceManager::SETTINGS_UPDATE_PROFILE_CHEATS },
	{ rsxppSettingsManager::UPDATE_PROFILE_COMMENTS,			ResourceManager::SETTINGS_UPDATE_PROFILE_COMMENTS },
	{ rsxppSettingsManager::SHOW_CLIENT_NEW_VER,				ResourceManager::SETTINGS_SHOW_CLIENT_NEW_VER },
	{ rsxppSettingsManager::SHOW_MYINFO_NEW_VER,				ResourceManager::SETTINGS_SHOW_USER_INFO_NEW_VER },
	{ rsxppSettingsManager::SHOW_IPWATCH_NEW_VER,				ResourceManager::SETTINGS_SHOW_IPWATCH_NEW_VER },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT RSXAdvanced::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_RSXADVANCED_BOOLEANS), true);

	CUpDownCtrl spin;
	attach_spin(IDC_MYINFO_COUNTER_SPIN,			spin, 0, 99);
	attach_spin(IDC_CTE_COUNTER_SPIN,				spin, 0, 99);
	attach_spin(IDC_PM_COUNTER_SPIN,				spin, 0, 99);
	attach_spin(IDC_MYINFO_TIME_SPIN,				spin, 200, 9000);
	attach_spin(IDC_CTE_TIME_SPIN,					spin, 200, 9000);
	attach_spin(IDC_PM_TIME_SPIN,					spin, 200, 9000);
	return TRUE;
}

void RSXAdvanced::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_RSXADVANCED_BOOLEANS), true);
}