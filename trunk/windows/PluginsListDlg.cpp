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
#include "../client/PluginsManager.h"

#include "PluginsListDlg.h"

LRESULT PluginsListDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;

	ctrlList.Attach(GetDlgItem(IDC_PLUGINS_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, (rc.Width() / 6), 0);
	ctrlList.InsertColumn(1, _T("Version"), LVCFMT_LEFT, (rc.Width() / 6), 0);
	ctrlList.InsertColumn(2, _T("Author"), LVCFMT_LEFT, (rc.Width() / 6), 0);
	ctrlList.InsertColumn(3, _T("Description"), LVCFMT_LEFT, (rc.Width() / 2)-18, 0);
	ctrlList.SetExtendedListViewStyle(/*LVS_EX_INFOTIP | */LVS_EX_FULLROWSELECT);

	const PluginsManager::Plugins& list = PluginsManager::getInstance()->getPlugins();
	for(PluginsManager::Plugins::const_iterator i = list.begin(); i != list.end(); ++i) {
		TStringList strings;
		strings.push_back((*i)->getName());
		strings.push_back((*i)->getVersion());
		strings.push_back((*i)->getAuthor());
		strings.push_back((*i)->getDescription());
		ctrlList.insert(ctrlList.GetItemCount(), strings, NULL, NULL);
	}
	return 0;
}

/**
 * @file
 * $Id$
 */