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
#include "../client/rsxppSettingsManager.h"
#include "../client/PluginsManager.h"

#include "ExtensionsCfgPage.h"
#include "LineDlg.h"

LRESULT ExtensionsCfgPage::onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CRect rc;

	ctrlList.Attach(GetDlgItem(IDC_EXTENSIONS_CFG));
	ctrlList.GetClientRect(rc);
	ctrlList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, rc.Width() / 3, 0);
	ctrlList.InsertColumn(1, _T("Value"), LVCFMT_LEFT, rc.Width() / 2, 0);
	ctrlList.SetExtendedListViewStyle(/*LVS_EX_CHECKBOXES |*/ LVS_EX_FULLROWSELECT);

	const StringMap& st = rsxppSettingsManager::getInstance()->getExtSettings();
	TStringList l;
	for(StringMap::const_iterator i = st.begin(); i != st.end(); ++i) {
		l.push_back(Text::toT(i->first));
		l.push_back(Text::toT(i->second));
		ctrlList.insert(l);
		l.clear();
	}
	return 0;
}

LRESULT ExtensionsCfgPage::onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& /*bHandled*/) {
	if(ctrlList.GetSelectedCount() == 1) {
		int sel = ctrlList.GetSelectedIndex();
		TCHAR buf[4096];
		ctrlList.GetItemText(sel, 1, buf, 4096);

		LineDlg dlg;
		dlg.line = buf;
		dlg.title = _T("Change Setting");
		if(dlg.DoModal() == IDOK) {
			ctrlList.SetItemText(sel, 1, dlg.line.c_str());
		}
	}
	return 0;
}

void ExtensionsCfgPage::write() {
	int items = ctrlList.GetItemCount();
	TCHAR buf[4096];

	rsxppSettingsManager::getInstance()->lock();
	for(int i = 0; i < items; ++i) {
		ctrlList.GetItemText(i, 0, buf, 4096);
		string name = Text::fromT(buf);
		ctrlList.GetItemText(i, 1, buf, 4096);
		string value = Text::fromT(buf);

		rsxppSettingsManager::getInstance()->setExtSetting(name, value);
	}
	rsxppSettingsManager::getInstance()->unlock();
	PluginsManager::getInstance()->plugEvent(DCPP_CFG_CHANGED, 0, 0, 0);
}

/**
 * @file
 * $Id$
 */