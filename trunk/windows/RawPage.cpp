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

#include "RawPage.h"
#include "LineDlg.h"
#include "RawDlg.h"

#include "../client/Util.h"
#include "../client/SettingsManager.h"
#include "../client/version.h"
#include "../rsx/RsxUtil.h"

PropPage::TextItem RawPage::texts[] = {
	{ IDC_RAW_PAGE_ACTION_CTN, ResourceManager::ACTION },
	{ IDC_MOVE_RAW_UP, ResourceManager::MOVE_UP },
	{ IDC_MOVE_RAW_DOWN, ResourceManager::MOVE_DOWN },
	{ IDC_ADD_ACTION, ResourceManager::ADD },
	{ IDC_RENAME_ACTION, ResourceManager::RENAME },
	{ IDC_REMOVE_ACTION, ResourceManager::REMOVE },
	{ IDC_ADD_RAW, ResourceManager::ADD },
	{ IDC_CHANGE_RAW, ResourceManager::SETTINGS_CHANGE },
	{ IDC_REMOVE_RAW, ResourceManager::REMOVE },
	{ 0, ResourceManager::SETTINGS_AUTO_AWAY }
};

LRESULT RawPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	PropPage::translate((HWND)(*this), texts);

	CRect rc1, rc2;
	ctrlAction.Attach(GetDlgItem(IDC_RAW_PAGE_ACTION));
	ctrlAction.GetClientRect(rc1);
	ctrlAction.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, rc1.Width() - 20, 0);
	ctrlAction.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	Action::List lst = RawManager::getInstance()->getActionList();
	for(Action::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
		addEntryAction(i->first, i->second->getName(), i->second->getActif(), ctrlAction.GetItemCount());
	}

	ctrlRaw.Attach(GetDlgItem(IDC_RAW_PAGE_RAW));
	ctrlRaw.GetClientRect(rc2);
	ctrlRaw.InsertColumn(0, _T("Dummy"), LVCFMT_LEFT, rc2.Width() - 20, 0);
	ctrlRaw.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	nosave = false;
	return TRUE;
}

LRESULT RawPage::onAddAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	try {
		LineDlg name;
		name.title = TSTRING(ADD_ACTION);
		name.description = TSTRING(NAME);
		name.line = TSTRING(ACTION);
		if(name.DoModal(m_hWnd) == IDOK) {
			addEntryAction(RawManager::getInstance()->addAction(0,	Text::fromT(name.line),	true), Text::fromT(name.line), true, ctrlAction.GetItemCount());
		}
	} catch(const Exception& e) {
		MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
	}

	return 0;
}

LRESULT RawPage::onRenameAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	TCHAR buf[MAX_PATH];
	LVITEM item;
	::ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.cchTextMax = sizeof(buf);
	item.pszText = buf;

	int i = -1;
	while((i = ctrlAction.GetNextItem(i, LVNI_SELECTED)) != -1) {
		item.iItem = i;
		item.iSubItem = 0;
		ctrlAction.GetItem(&item);

		try {
			LineDlg name;
			name.title = TSTRING(ADD_ACTION);
			name.description = TSTRING(NAME);
			name.line = tstring(buf);
			if(name.DoModal(m_hWnd) == IDOK) {
				if (Util::stricmp(buf, name.line) != 0) {
					RawManager::getInstance()->renameAction(Text::fromT(buf), Text::fromT(name.line));
					ctrlAction.SetItemText(i, 0, name.line.c_str());
				} else {
					MessageBox(CTSTRING(SKIP_RENAME_ACTION), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONINFORMATION | MB_OK);
				}
			}
		} catch(const Exception& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}

	return 0;
}

LRESULT RawPage::onRemoveAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAction.GetSelectedCount() == 1) {
		int i = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		RawManager::getInstance()->removeAction(ctrlAction.GetItemData(i));
		ctrlAction.DeleteItem(i);
	}

	return 0;
}

LRESULT RawPage::onAddRaw(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAction.GetSelectedCount() == 1) {
		int i = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		try {
			RawDlg raw;
			if(raw.DoModal(m_hWnd) == IDOK) {
				addEntryRaw(RawManager::getInstance()->addRaw(
					ctrlAction.GetItemData(i), raw.name, raw.raw, raw.time, raw.useLua), ctrlRaw.GetItemCount()
				);
			}
		} catch(const Exception& e) {
			MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		}
	}

	return 0;
}

LRESULT RawPage::onChangeRaw(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAction.GetSelectedCount() == 1) {
		int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		if(ctrlRaw.GetSelectedCount() == 1) {
			int i = ctrlRaw.GetNextItem(-1, LVNI_SELECTED);
			Action::Raw ra;
			RawManager::getInstance()->getRawItem(ctrlAction.GetItemData(j), ctrlRaw.GetItemData(i), ra);

			try {
				RawDlg raw;
				string name = ra.getName();
				raw.name = name;
				raw.raw = ra.getRaw();
				raw.time = ra.getTime();
				raw.useLua = ra.getLua();
				if(raw.DoModal() == IDOK) {
					RawManager::getInstance()->changeRaw(ctrlAction.GetItemData(j), name, raw.name, raw.raw, raw.time, raw.useLua);
					ctrlRaw.SetItemText(i, 0, Text::toT(raw.name).c_str());
				}
			} catch(const Exception& e) {
				MessageBox(Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
			}
		}
	}

	return 0;
}

LRESULT RawPage::onRemoveRaw(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAction.GetSelectedCount() == 1) {
		int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		if(ctrlRaw.GetSelectedCount() == 1) {
			int i = ctrlRaw.GetNextItem(-1, LVNI_SELECTED);
			RawManager::getInstance()->removeRaw(ctrlAction.GetItemData(j), ctrlRaw.GetItemData(i));
			ctrlRaw.DeleteItem(i);
		}
	}

	return 0;
}

LRESULT RawPage::onMoveRawUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAction.GetSelectedCount() == 1) {
		int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		int i = ctrlRaw.GetSelectedIndex();
		if(i != -1 && i != 0) {
			int n = ctrlRaw.GetItemData(i);
			RawManager::getInstance()->moveRaw(ctrlAction.GetItemData(j), n, -1/*, i*/);
			ctrlRaw.SetRedraw(FALSE);
			ctrlRaw.DeleteItem(i);
			Action::Raw ra;
			RawManager::getInstance()->getRawItem(ctrlAction.GetItemData(j), n, ra);
			addEntryRaw(ra, i-1);
			ctrlRaw.SelectItem(i-1);
			ctrlRaw.EnsureVisible(i-1, FALSE);
			ctrlRaw.SetRedraw(TRUE);
		}
	}
	return 0;
}

LRESULT RawPage::onMoveRawDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlAction.GetSelectedCount() == 1) {
		int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		int i = ctrlRaw.GetSelectedIndex();
		if(i != -1 && i != (ctrlRaw.GetItemCount()-1) ) {
			int n = ctrlRaw.GetItemData(i);
			RawManager::getInstance()->moveRaw(ctrlAction.GetItemData(j), n, 1/*, i*/);
			ctrlRaw.SetRedraw(FALSE);
			ctrlRaw.DeleteItem(i);
			Action::Raw ra;
			RawManager::getInstance()->getRawItem(ctrlAction.GetItemData(j), n, ra);
			addEntryRaw(ra, i+1);
			ctrlRaw.SelectItem(i+1);
			ctrlRaw.EnsureVisible(i+1, FALSE);
			ctrlRaw.SetRedraw(TRUE);
		}
	}
	return 0;
}

void RawPage::addEntryAction(int id, const string name, bool actif, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(name));
	int i = ctrlAction.insert(pos, lst, 0, (LPARAM)id);
	ctrlAction.SetCheckState(i, actif);
}

void RawPage::addEntryRaw(const Action::Raw& ra, int pos) {
	TStringList lst;

	lst.push_back(Text::toT(ra.getName()));
	int i = ctrlRaw.insert(pos, lst, 0, (LPARAM)ra.getId());
	ctrlRaw.SetCheckState(i, ra.getActif());
}

LRESULT RawPage::onItemChangedRaw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	gotFocusOnRaw = (l->uNewState & LVIS_FOCUSED) || ((l->uNewState & LVIS_STATEIMAGEMASK) && ctrlRaw.GetSelectedIndex() != -1);
	::EnableWindow(GetDlgItem(IDC_CHANGE_RAW), gotFocusOnRaw && gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_REMOVE_RAW), gotFocusOnRaw && gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_MOVE_RAW_UP), gotFocusOnRaw && gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_MOVE_RAW_DOWN), gotFocusOnRaw && gotFocusOnAction); 
	return 0;
}
LRESULT RawPage::onItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NMITEMACTIVATE* l = (NMITEMACTIVATE*)pnmh;
	gotFocusOnAction = (l->uNewState & LVIS_FOCUSED) || ((l->uNewState & LVIS_STATEIMAGEMASK) && ctrlAction.GetSelectedIndex() != -1);
	::EnableWindow(GetDlgItem(IDC_RENAME_ACTION), gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_REMOVE_ACTION), gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_RAW_PAGE_RAW), gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_ADD_RAW), gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_CHANGE_RAW), gotFocusOnRaw && gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_REMOVE_RAW), gotFocusOnRaw && gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_MOVE_RAW_UP), gotFocusOnRaw && gotFocusOnAction);
	::EnableWindow(GetDlgItem(IDC_MOVE_RAW_DOWN), gotFocusOnRaw && gotFocusOnAction); 

	if(!nosave && l->iItem != -1 && ((l->uNewState & LVIS_STATEIMAGEMASK) == (l->uOldState & LVIS_STATEIMAGEMASK))) {
		int j;
		for(j = 0; j < ctrlRaw.GetItemCount(); j++) {
			RawManager::getInstance()->setActifRaw(ctrlAction.GetItemData(l->iItem), ctrlRaw.GetItemData(j), RsxUtil::toBool(ctrlRaw.GetCheckState(j)));
		}
		ctrlRaw.SetRedraw(FALSE);
		ctrlRaw.DeleteAllItems();

		if(ctrlAction.GetSelectedCount() == 1) {
			Action::RawsList lst = RawManager::getInstance()->getRawList(ctrlAction.GetItemData(l->iItem));

			for(Action::RawsList::const_iterator i = lst.begin(); i != lst.end(); ++i) {
				const Action::Raw& ra = *i;	
				addEntryRaw(ra, ctrlRaw.GetItemCount());
			}
		}
		ctrlRaw.SetRedraw(TRUE);
	}
	return 0;
}

void RawPage::write() {
	int i;
	for(i = 0; i < ctrlAction.GetItemCount(); i++) {
		RawManager::getInstance()->setActifAction(ctrlAction.GetItemData(i), RsxUtil::toBool(ctrlAction.GetCheckState(i)));
	}
	if(ctrlAction.GetSelectedCount() == 1) {
		int j = ctrlAction.GetNextItem(-1, LVNI_SELECTED);
		int l;
		for(l = 0; l < ctrlRaw.GetItemCount(); l++) {
			RawManager::getInstance()->setActifRaw(ctrlAction.GetItemData(j), ctrlRaw.GetItemData(l), RsxUtil::toBool(ctrlRaw.GetCheckState(l)));
		}
	}
	RawManager::getInstance()->saveActionRaws();
}