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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"
#include "../rsx/rsx-settings/rsx-SettingsManager.h"
#include "../client/IgnoreManager.h"

#include "RSX-Page.h"
#include "CommandDlg.h"
#include "../client/version.h"

#include "WinUtil.h"
#include "MainFrm.h"

PropPage::TextItem RSX::texts[] = {
	{ IDC_IGNORE_ADD,			ResourceManager::ADD },
	{ IDC_IGNORE_REMOVE,		ResourceManager::REMOVE },
	{ IDC_IGNORE_CLEAR,			ResourceManager::IGNORE_CLEAR },
	{ IDC_MISC_IGNORE,			ResourceManager::IGNORED_USERS },
	{ IDC_USE_REGEXP_OR_WILD,	ResourceManager::USE_REGEXP_OR_WC },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

PropPage::Item RSX::items[] = {
	{ IDC_USE_REGEXP_OR_WILD,	RSXSettingsManager::IGNORE_USE_REGEXP_OR_WC,	PropPage::T_BOOL_RSX },
	{ 0, 0, PropPage::T_END }
};
RSX::ListItem RSX::listItems[] = {
	{ RSXSettingsManager::PARTIAL_FILE_SHARING,	ResourceManager::PARTIAL_FILE_SHARING },
	{ RSXSettingsManager::USE_FILTER_FAV, ResourceManager::USE_FILTER_FAV },
	{ RSXSettingsManager::USE_HL_FAV, ResourceManager::USE_HL_FAV },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT RSX::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);
	PropPage::read((HWND)*this, items, listItems, GetDlgItem(IDC_RSX_BOOLEANS), true);

	CRect rc;

	ignoreListCtrl.Attach(GetDlgItem(IDC_IGNORELIST));
	ignoreListCtrl.GetClientRect(rc);
	ignoreListCtrl.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, (rc.Width() - 17), 0);
	ignoreListCtrl.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	ignoreList = IgnoreManager::getInstance()->getIgnoredUsers();
	for(TStringHash::iterator i = ignoreList.begin(); i != ignoreList.end(); ++i) {
		ignoreListCtrl.insert(ignoreListCtrl.GetItemCount(), *i);
	}

	ctrlPrio.Attach(GetDlgItem(IDC_STARTUP_PRIO));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_REALTIME));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_HIGH));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_ABOVE));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_NORMAL));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_BELOW));
	ctrlPrio.AddString(CTSTRING(MENU_PRIO_IDLE));
	ctrlPrio.SetCurSel(RSXSETTING(DEFAULT_PRIO));
	return TRUE;
}

void RSX::write() {
	PropPage::write((HWND)*this, items, listItems, GetDlgItem(IDC_RSX_BOOLEANS), true);
	IgnoreManager::getInstance()->putIgnoredUsers(ignoreList);
	RSXSettingsManager::getInstance()->set(RSXSettingsManager::DEFAULT_PRIO, ctrlPrio.GetCurSel());
}

LRESULT RSX::onEditChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDC_IGNORELIST_EDIT)
		::EnableWindow(GetDlgItem(IDC_IGNORE_ADD), (::GetWindowTextLength(GetDlgItem(IDC_IGNORELIST_EDIT)) > 0));
	return 0;
}

LRESULT RSX::onItemchanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(GetDlgItem(IDC_IGNORE_REMOVE), (lv->uNewState & LVIS_FOCUSED));
	return 0;
}

LRESULT RSX::onIgnoreAdd(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	TCHAR buf[256];
	if(GetDlgItemText(IDC_IGNORELIST_EDIT, buf, 256)) {
		pair<TStringHashIter, bool> p = ignoreList.insert(buf);
	
		if(p.second) {
			ignoreListCtrl.insert(ignoreListCtrl.GetItemCount(), buf);
		} else {
			MessageBox(CTSTRING(ALREADY_IGNORED), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK);
		}
	}

	SetDlgItemText(IDC_IGNORELIST_EDIT, _T(""));
	return 0;
}

LRESULT RSX::onIgnoreRemove(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	int i = -1;
	
	TCHAR buf[256];

	while((i = ignoreListCtrl.GetNextItem(-1, LVNI_SELECTED)) != -1) {
		ignoreListCtrl.GetItemText(i, 0, buf, 256);

		ignoreList.erase(buf);
		ignoreListCtrl.DeleteItem(i);
	}

	return 0;
}

LRESULT RSX::onIgnoreClear(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	ignoreListCtrl.DeleteAllItems();
	ignoreList.clear();
	return 0;
}