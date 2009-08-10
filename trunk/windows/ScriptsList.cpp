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
#include "resource.h"

#include "../client/DCPlusPlus.h"

#include "ScriptsList.h"
#include "../client/LuaScript.h"
#include "../client/ScriptManager.h"

LRESULT ScriptsListPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;

	ctrlList.Attach(GetDlgItem(IDC_SCRIPTS_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Script"), LVCFMT_LEFT, rc.Width() - 70, 0);
	ctrlList.InsertColumn(1, _T("Loaded"), LVCFMT_LEFT, 50, 0);

	ctrlList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	const ScriptManager::Scripts& list = ScriptManager::getInstance()->getScripts();
	int count = 0;
	TStringList l;
	for(ScriptManager::Scripts::const_iterator i = list.begin(); i != list.end(); ++i, ++count) {
		l.push_back((*i)->getFileName());
		l.push_back((*i)->loaded ? _T("*") : Util::emptyStringT);

		int item = ctrlList.insert(l, 0, 0);
		ctrlList.SetItemData(item, (DWORD_PTR)(*i)->path.c_str());
		ctrlList.SetCheckState(item, (*i)->enabled);
		l.clear();
	}
	::SetWindowText(GetDlgItem(IDC_STATIC1), Text::toT("Scripts Dir: " + Util::getPath(Util::PATH_GLOBAL_CONFIG) + "LuaScripts").c_str());
	::SetWindowText(GetDlgItem(IDC_STATIC2), Text::toT("Scripts Count: " + Util::toString(count)).c_str());
	return 0;
}

void ScriptsListPage::write() {
	int items = ctrlList.GetItemCount();
	ScriptManager::Scripts& list = ScriptManager::getInstance()->getScripts();

	for(int i = 0; i < items; ++i) {
		TCHAR* path = (TCHAR*)ctrlList.GetItemData(i);
		bool state = ctrlList.GetCheckState(i) != 0;
		for(ScriptManager::Scripts::iterator j = list.begin(); j != list.end(); ++j) {
			if(stricmp(path, (*j)->path) == 0) {
				(*j)->enabled = state;
				break;
			}
		}
	}
}

/**
 * @file
 * $Id$
 */