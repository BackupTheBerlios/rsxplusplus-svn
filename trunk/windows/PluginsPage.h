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

#ifndef RSXPLUSPLUS_PLUGINS_PAGE
#define RSXPLUSPLUS_PLUGINS_PAGE

#include "PropPage.h"
#include "DlgTabCtrl.h"

class PluginsPage : public CPropertyPage<IDD_PLUGINS_PAGE>, public PropPage {
public:
	PluginsPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + _T("Plugins")).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	}
	~PluginsPage() { free(title); }

	BEGIN_MSG_MAP(PluginsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

protected:
	//std::map<int, HWND> plugins;

	//HWND addPage(int pId, int resId, const tstring& tabName, HMODULE hInst);

	TCHAR* title;
	//CDialogTabCtrl ctrlTabs;
	static Item items[];
};

#endif //RSXPAGE_H