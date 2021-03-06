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

#ifndef IPWATCH_DLG
#define IPWATCH_DLG

#include "CRawCombo.h"

class IPWatchDlg : public CDialogImpl<IPWatchDlg> {
public:
	tstring pattern, cheat, isp;
	int task, action, mode, matchType;
	bool display;

	enum { IDD = IDD_ADD_IPWATCH };

	BEGIN_MSG_MAP(FilterPageDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_WATCH_ACTION, onAction)
		COMMAND_ID_HANDLER(IDC_IPW_MODE, onAction)
	END_MSG_MAP()

	IPWatchDlg() : mode(0), pattern(_T("0.0.0.0")), cheat(_T("Forbidden IP!")), task(0), action(0), display(true), matchType(0), isp(_T("")) { };
	~IPWatchDlg() {
		cAction.Detach();
		cTask.Detach();
		cMode.Detach();
		cMatchType.Detach();
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		cPattern.SetFocus();
		return FALSE;
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CEdit cPattern;
	CComboBox cTask, cMode, cMatchType;
	CRawCombo cAction;
	CButton cDisplay;
	void fixControls();
};
#endif //IPWATCH_DLG

/**
 * @file
 * $Id: IPWatchDlg.h 230 2011-01-30 18:15:33Z adrian_007 $
 */