/*
 * Copyright (C) 2007-2011 adrian_007, adrian-007 on o2 point pl
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

#ifndef RSXPAGE_H
#define RSXPAGE_H

#include "PropPage.h"
#include "ExListViewCtrl.h"

class RSXPage : public CPropertyPage<IDD_RSX>, public PropPage {
public:

	RSXPage(SettingsManager *s) : PropPage(s) {
		title = _tcsdup(TSTRING(SETTINGS_RSX).c_str());
		SetTitle(title);
		m_psp.dwFlags |= PSP_RTLREADING;
	}
	~RSXPage() {
		free(title);
	}

	BEGIN_MSG_MAP_EX(RSXPage)
		MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
	END_MSG_MAP()

	LRESULT onInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

//private:
protected:
	TCHAR* title;

	static Item items[];
	static TextItem texts[];
	static ListItem listItems[];

	CComboBox ctrlPrio;
	CComboBox ctrlDictionary;
	int CurSel;
};

#endif //RSXPAGE_H