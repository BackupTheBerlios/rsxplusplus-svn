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

#ifndef RAWPAGE_H
#define RAWPAGE_H

#include "PropPage.h"
#include "ExListViewCtrl.h"
#include "../client/RawManager.h"

class RawPage : public CPropertyPage<IDD_RAW_PAGE>, public PropPage {
public:
	RawPage(SettingsManager *s) : PropPage(s), expanded(false) { 
		title = _tcsdup((TSTRING(SETTINGS_RSX) + _T('\\') + TSTRING(SETTINGS_FAKEDETECT) + _T('\\') + TSTRING(SETTINGS_ACTION_RAW)).c_str());
		SetTitle(title);
	};

	~RawPage() {
		//ctrlList.Detach();
		free(title);
	};

	BEGIN_MSG_MAP(RawPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_EXPAND, onExpand)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_PARENT_ADD, onParentAdd)
		COMMAND_ID_HANDLER(IDC_PARENT_EDIT, onParentEdit)
		COMMAND_ID_HANDLER(IDC_CHILD_ADD, onChildAdd)
		COMMAND_ID_HANDLER(IDC_CHILD_EDIT, onChildEdit)
		COMMAND_ID_HANDLER(IDC_CHILD_UP, onChildUp)
		COMMAND_ID_HANDLER(IDC_CHILD_DOWN, onChildDown)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onExpand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onParentAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onParentEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChildAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChildEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChildUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChildDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Common PropPage interface
	PROPSHEETPAGE *getPSP() { return (PROPSHEETPAGE *)*this; }
	void write();

protected:
	bool expanded;

	HTREEITEM addAction(const Action* action);
	void addRaw(HTREEITEM action, const Raw* raw);
	void updateRaw(HTREEITEM item, Raw* raw);

	static TextItem texts[];
	CTreeViewCtrl ctrlList;

	TCHAR* title;
};
#endif //RAWPAGE_H