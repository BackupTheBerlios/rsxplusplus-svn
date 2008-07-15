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

#include "ScriptsList.h"
#include "../client/ScriptManager.h"

LRESULT ScriptsListDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;

	ctrlList.Attach(GetDlgItem(IDC_SCRIPTS_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Script"), LVCFMT_LEFT, rc.Width() - 20, 0);
	ctrlList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	const ScriptManager::ScriptsList& list = ScriptManager::getInstance()->getScriptsList();
	int count = 0, active = 0;
	for(ScriptManager::ScriptsList::const_iterator i = list.begin(); i != list.end(); ++i, ++count) {
		int item = ctrlList.insert(ctrlList.GetItemCount(), Text::toT(i->second), 0, 0);
		ctrlList.SetCheckState(item, i->first);
		if(i->first)
			++active;
	}
	::SetWindowText(GetDlgItem(IDC_STATIC1), Text::toT("Scripts Dir: " + Util::getConfigPath() + "scripts\\").c_str());
	::SetWindowText(GetDlgItem(IDC_STATIC2), Text::toT("Scripts Count: " + Util::toString(count)).c_str());
	::SetWindowText(GetDlgItem(IDC_STATIC3), Text::toT("Active Scripts: " + Util::toString(active)).c_str());
	return 0;
}

LRESULT ScriptsListDlg::onSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int items = ctrlList.GetItemCount();
	TCHAR buf[1024];
	ScriptManager::ScriptsList& list = ScriptManager::getInstance()->getScriptsList();
	list.clear();
	for(int i = 0; i < items; ++i) {
		ctrlList.GetItemText(i, 0, buf, 1024);
		bool state = ctrlList.GetCheckState(i) != 0;
		list.push_back(make_pair(state, Text::fromT(buf)));
	}
	RSXSettingsManager::getInstance()->save();
	EndDialog(0);
	return 0;
}

/**
 * @file
 * $Id$
 */