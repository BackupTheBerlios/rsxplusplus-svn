/*
 * Copyright (C) 2007-2010 adrian_007, adrian-007 on o2 point pl
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
	ctrlList.Attach(GetDlgItem(IDC_LIST));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Plugin"), LVCFMT_LEFT, rc.Width() / 5, 0);
	ctrlList.InsertColumn(1, _T("Description"), LVCFMT_LEFT, rc.Width() / 4, 0);
	ctrlList.InsertColumn(2, _T("Author"), LVCFMT_LEFT, rc.Width() / 5, 0);
	ctrlList.InsertColumn(3, _T("Website"), LVCFMT_LEFT, rc.Width() / 5, 0);
	ctrlList.InsertColumn(4, _T("Version"), LVCFMT_LEFT, rc.Width() / 5 - 50, 0);
	ctrlList.SetExtendedListViewStyle(/*LVS_EX_CHECKBOXES |*/ LVS_EX_FULLROWSELECT);

	std::list<dcppPluginInformation*> plugins;
	PluginsManager::getInstance()->getPluginsInfo(plugins);
	TStringList list;
	wchar_t buf[64];
	for(std::list<dcppPluginInformation*>::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		list.push_back(Text::toT((*i)->name ? (*i)->name : "N/A"));
		list.push_back(Text::toT((*i)->description ? (*i)->description : "N/A"));
		list.push_back(Text::toT((*i)->author ? (*i)->author : "N/A"));
		list.push_back(Text::toT((*i)->website ? (*i)->website : "N/A"));
		swprintf(buf, 64, L"%d.%d.%d.%d", VER_MAJ((*i)->version), VER_MIN((*i)->version), VER_REV((*i)->version), VER_BLD((*i)->version));
		list.push_back(buf);
		ctrlList.insert(list);
		list.clear();
	}
	return 0;
}

/**
 * @file
 * $Id$
 */