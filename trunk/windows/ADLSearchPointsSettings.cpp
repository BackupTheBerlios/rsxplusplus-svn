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
#include "resource.h"

#include "../client/DCPlusPlus.h"
#include "../client/RawManager.h"

#include "../rsx/rsx-settings/rsx-SettingsManager.h"

#include "ADLSearchPointsSettings.h"
#include "ADLSearchPoints.h"

LRESULT ADLSearchPointsSettings::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;
	ctrlList.Attach(GetDlgItem(IDC_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Value"), LVCFMT_LEFT, (rc.Width() / 2) - 40, 0);
	ctrlList.InsertColumn(1, _T("Action"), LVCFMT_LEFT, (rc.Width() / 2) + 20, 0);
	ctrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	const IntMap& list = RawManager::getInstance()->getADLPoints();
	for(IntMap::const_iterator i = list.begin(); i != list.end(); ++i) {
		TStringList strings;
		strings.push_back(Util::toStringW(i->first));
		strings.push_back(RawManager::getInstance()->getNameActionId(i->second));
		int pos = ctrlList.insert(strings);
		ctrlList.SetItemData(pos, (DWORD)i->second);
	}
	SetDlgItemInt(IDC_EDIT, RSXSETTING(MIN_POINTS_TO_DISPLAY_CHEAT));
	return FALSE;
}

LRESULT ADLSearchPointsSettings::onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PointValue dlg;
	if(dlg.DoModal() == IDOK) {
		if(ctrlList.find(Util::toStringW(dlg.value)) == -1) {
			TStringList lst;
			lst.push_back(Util::toStringW(dlg.value));
			lst.push_back(RawManager::getInstance()->getNameActionId(dlg.actionId));
			int pos = ctrlList.insert(lst);
			ctrlList.SetItemData(pos, (DWORD)dlg.actionId);
		} else {
			MessageBox(_T("This value already exist!"));
		}
	}
	return 0;
}

LRESULT ADLSearchPointsSettings::onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		ctrlList.DeleteItem(ctrlList.GetNextItem(-1, LVNI_SELECTED));
	}
	return 0;
}

LRESULT ADLSearchPointsSettings::onClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		int count = ctrlList.GetItemCount();
		IntMap tmp;
		TCHAR buf[512];
		for(int i = 0; i < count; ++i) {
			ctrlList.GetItemText(i, 0, buf, 512);
			int v = Util::toInt(Text::fromT(buf));
			int a = (int)ctrlList.GetItemData(i);
			tmp.insert(make_pair(v, a));
		}
		RawManager::getInstance()->updateADLPoints(tmp);
		int min_p = GetDlgItemInt(IDC_EDIT);
		RSXS_SET(MIN_POINTS_TO_DISPLAY_CHEAT, min_p);
	}
	ctrlList.Detach();
	EndDialog(wID);
	return 0;
}


/**
 * @file
 * $Id$
 */