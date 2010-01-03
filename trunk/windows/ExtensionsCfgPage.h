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

#ifndef RSXPLUSPLUS_EXTENSIONS_CFG_PAGE
#define RSXPLUSPLUS_EXTENSIONS_CFG_PAGE

#include "ExListViewCtrl.h"
#include "PropPage.h"

class ExtensionsCfgPage : public CPropertyPage<IDD_EXTENSIONS_CFG>, public PropPage {
public:
	ExtensionsCfgPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + _T("Add-ons Settings")).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	}
	~ExtensionsCfgPage() { 
		free(title);
		ctrlList.Detach();

	}

	BEGIN_MSG_MAP(ExtensionsCfgPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		NOTIFY_HANDLER(IDC_EXTENSIONS_CFG, NM_DBLCLK, onDblClick)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onDblClick(int /*idCtrl*/, LPNMHDR /* pnmh */, BOOL& bHandled);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();
private:
	TCHAR* title;
	ExListViewCtrl ctrlList;
};
#endif

/**
 * @file
 * $Id$
 */