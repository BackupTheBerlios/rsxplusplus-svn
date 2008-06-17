/*
 * Copyright (C) 2006-2008 Crise, crise@mail.berlios.de
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
#include "Resource.h"
#include "WinUtil.h"

#include "DetectionEntryDlg.h"
#include "ParamDlg.h"

#undef GET_TEXT
#define GET_TEXT(id, var) \
	GetDlgItemText(id, buf, 1024); \
	var = Text::fromT(buf);

#undef ATTACH
#define ATTACH(id, var) var.Attach(GetDlgItem(id))

LRESULT DetectionEntryDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	if(origId < 1) {
		::EnableWindow(GetDlgItem(IDC_BACK), false);
		::EnableWindow(GetDlgItem(IDC_NEXT), false);
	}

	ATTACH(IDC_NAME, ctrlName);
	ATTACH(IDC_COMMENT, ctrlComment);
	ATTACH(IDC_CHEAT, ctrlCheat);
	ATTACH(IDC_PARAMS, ctrlParams);
	ATTACH(IDC_RAW, ctrlRaw);
	ATTACH(IDC_LEVEL, ctrlLevel);

	CRect rc;
	ctrlParams.GetClientRect(rc);
	ctrlParams.InsertColumn(0, CTSTRING(SETTINGS_NAME), LVCFMT_LEFT, rc.Width() / 10, 0);
	ctrlParams.InsertColumn(1, CTSTRING(REGEXP), LVCFMT_LEFT, ((rc.Width() / 10) * 9) - 17, 1);
	ctrlParams.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	createList();
	for(Iter i = idAction.begin(); i != idAction.end(); ++i) {
		ctrlRaw.AddString(RawManager::getInstance()->getNameActionId(i->second).c_str());
	}

	ctrlLevel.AddString(_T("Green"));
	ctrlLevel.AddString(_T("Yellow"));
	ctrlLevel.AddString(_T("Red"));

	updateControls();

	CenterWindow(GetParent());
	return FALSE;
}

LRESULT DetectionEntryDlg::onAdd(WORD , WORD , HWND , BOOL& ) {
	ParamDlg dlg;

	if(dlg.DoModal() == IDOK) {
		if(ctrlParams.find(Text::toT(dlg.name)) == -1) {
			TStringList lst;
			lst.push_back(Text::toT(dlg.name));
			lst.push_back(Text::toT(dlg.regexp));
			ctrlParams.insert(lst);
		} else {
			::MessageBox((HWND)*this, CTSTRING(PARAM_EXISTS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK);
		}
	}
	return 0;
}

LRESULT DetectionEntryDlg::onItemchangedDirectories(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	NM_LISTVIEW* lv = (NM_LISTVIEW*) pnmh;
	::EnableWindow(GetDlgItem(IDC_CHANGE), (lv->uNewState & LVIS_FOCUSED));
	::EnableWindow(GetDlgItem(IDC_REMOVE), (lv->uNewState & LVIS_FOCUSED));
	return 0;
}

LRESULT DetectionEntryDlg::onChange(WORD , WORD , HWND , BOOL& ) {
	if(ctrlParams.GetSelectedCount() == 1) {
		int sel = ctrlParams.GetSelectedIndex();
		TCHAR buf[1024];
		ParamDlg dlg;
		ctrlParams.GetItemText(sel, 0, buf, 1024);
		dlg.name = Text::fromT(buf);
		ctrlParams.GetItemText(sel, 1, buf, 1024);
		dlg.regexp = Text::fromT(buf);

		if(dlg.DoModal() == IDOK) {
			int idx = ctrlParams.find(Text::toT(dlg.name));
			if(idx == -1 || idx == sel) {
				ctrlParams.SetItemText(sel, 0, Text::toT(dlg.name).c_str());
				ctrlParams.SetItemText(sel, 1, Text::toT(dlg.regexp).c_str());
			} else {
				::MessageBox((HWND)*this, CTSTRING(PARAM_EXISTS), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_OK);
			}
		}
	}
	return 0;
}

LRESULT DetectionEntryDlg::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlParams.GetSelectedCount() == 1) {
		ctrlParams.DeleteItem(ctrlParams.GetNextItem(-1, LVNI_SELECTED));
	}
	return 0;
}

LRESULT DetectionEntryDlg::onNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	updateVars();
	try {
		DetectionManager::getInstance()->updateDetectionItem(origId, curEntry);
	} catch(const Exception& e) {
		::MessageBox((HWND)*this, Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
		return 0;
	}

	if(DetectionManager::getInstance()->getNextDetectionItem(curEntry.Id, ((wID == IDC_NEXT) ? 1 : -1), curEntry) && idChanged) {
		idChanged = false;
		::EnableWindow(GetDlgItem(IDC_DETECT_ID), false);
		::EnableWindow(GetDlgItem(IDC_ID_EDIT), true);
	}

	origId = curEntry.Id;
	ctrlParams.DeleteAllItems();

	updateControls();
	return 0;
}

LRESULT DetectionEntryDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) { 
		updateVars();
		try {
			if(origId < 1) {
				DetectionManager::getInstance()->addDetectionItem(curEntry);
			} else {
				DetectionManager::getInstance()->updateDetectionItem(origId, curEntry);
			}
		} catch(const Exception& e) {
			::MessageBox((HWND)*this, Text::toT(e.getError()).c_str(), _T(APPNAME) _T(" ") _T(VERSIONSTRING), MB_ICONSTOP | MB_OK);
			return 0;
		}
	}

	EndDialog(wID);
	return 0;
}

void DetectionEntryDlg::updateVars() {
	TCHAR buf[1024];
	GET_TEXT(IDC_NAME, curEntry.name);
	GET_TEXT(IDC_COMMENT, curEntry.comment);
	GET_TEXT(IDC_CHEAT, curEntry.cheat);

	// params...
	int it = ctrlParams.GetItemCount();
	string name, regexp;
	curEntry.infMap.clear();
	for(int i = 0; i < it; ++i) {
		ctrlParams.GetItemText(i, 0, buf, 1024);
		name = Text::fromT(buf);
		ctrlParams.GetItemText(i, 1, buf, 1024);
		regexp = Text::fromT(buf);
		curEntry.infMap.push_back(make_pair(name, regexp));
	}

	if(idChanged) {
		GetDlgItemText(IDC_DETECT_ID, buf, 1024);
		uint32_t newId = Util::toUInt32(Text::fromT(buf).c_str());
		if(newId != origId) curEntry.Id = newId;
	}

	curEntry.checkMismatch = IsDlgButtonChecked(IDC_CHECK_MISMATCH) == BST_CHECKED;
	curEntry.isEnabled = IsDlgButtonChecked(IDC_ENABLE) == BST_CHECKED;
	curEntry.rawToSend = getIdAction(ctrlRaw.GetCurSel());
	curEntry.clientFlag = ctrlLevel.GetCurSel() + 1;
}

void DetectionEntryDlg::updateControls() {
	if(origId > 0) {
		ctrlName.SetWindowText(Text::toT(curEntry.name).c_str());
		ctrlComment.SetWindowText(Text::toT(curEntry.comment).c_str());
		ctrlCheat.SetWindowText(Text::toT(curEntry.cheat).c_str());

		// params...
		if(!curEntry.infMap.empty()) {
			TStringList cols;
			const DetectionEntry::StringMapV& lst = curEntry.infMap;
			for(DetectionEntry::StringMapV::const_iterator j = lst.begin(); j != lst.end(); ++j) {
				cols.push_back(Text::toT(j->first));
				cols.push_back(Text::toT(j->second));
				ctrlParams.insert(cols);
				cols.clear();
			}
		}

		SetDlgItemText(IDC_DETECT_ID, Util::toStringW(curEntry.Id).c_str());
	}

	CheckDlgButton(IDC_CHECK_MISMATCH, curEntry.checkMismatch ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ENABLE, curEntry.isEnabled ? BST_CHECKED : BST_UNCHECKED);
	ctrlRaw.SetCurSel(getId(curEntry.rawToSend));
	ctrlLevel.SetCurSel(curEntry.clientFlag - 1);
}