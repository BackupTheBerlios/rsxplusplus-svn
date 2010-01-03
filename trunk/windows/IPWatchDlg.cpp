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
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "IPWatchDlg.h"

#define ATTACH(id, var) var.Attach(GetDlgItem(id))
LRESULT IPWatchDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ATTACH(IDC_WATCH_IP, cPattern);
	ATTACH(IDC_WATCH_ACTION, cTask);
	ATTACH(IDC_WATCH_DISPLAY_CHEAT, cDisplay);
	ATTACH(IDC_IPW_MODE, cMode);
	ATTACH(IDC_IPW_MATCH_TYPE, cMatchType);

	cPattern.SetWindowText(pattern.c_str());
	::SetWindowText(GetDlgItem(IDC_WATCH_CHEAT), cheat.c_str());
	::SetWindowText(GetDlgItem(IDC_IPW_ISP), isp.c_str());
	cDisplay.SetCheck(display ?	BST_CHECKED : BST_UNCHECKED);

	cTask.AddString(_T("Protect"));
	cTask.AddString(_T("Ban"));
	cTask.AddString(_T("Notify"));
	cTask.AddString(_T("None"));
	cTask.SetCurSel(task);

	cMode.AddString(_T("IP"));
	cMode.AddString(_T("DNS/Host Name"));
	cMode.AddString(_T("IP Range"));
	cMode.SetCurSel(mode);

	cMatchType.AddString(_T("Wildcards"));
	cMatchType.AddString(_T("RegEx"));
	cMatchType.SetCurSel(matchType);

	cAction.attach(GetDlgItem(IDC_WATCH_ACTION_CMD), action);

	fixControls();
	CenterWindow(GetParent());
	return FALSE;
}

LRESULT IPWatchDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		TCHAR buf[512];
		if (cPattern.GetWindowTextLength() == 0) {
			MessageBox(CTSTRING(LINE_EMPTY));
			return 0;
		}

		GetDlgItemText(IDC_WATCH_IP, buf, 512);
		pattern = buf;
		GetDlgItemText(IDC_IPW_ISP, buf, 512);
		isp = buf;
		GetDlgItemText(IDC_WATCH_CHEAT, buf, 512);
		cheat = buf;

		task = cTask.GetCurSel();
		action = cAction.getActionId();
		display = cDisplay.GetCheck() == BST_CHECKED;
		matchType = cMatchType.GetCurSel();
		mode = cMode.GetCurSel();
	}
	EndDialog(wID);
	return 0;
}

LRESULT IPWatchDlg::onAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

void IPWatchDlg::fixControls() {
	switch(cTask.GetCurSel()) {
		case 0: { //protect
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), false);
			break; 
		}
		case 1: { //ban
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), true);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), true);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), true);
			break; 
		}
		case 2: { //notify
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), true);
			break; 
		}
		case 3: {
			::EnableWindow(GetDlgItem(IDC_WATCH_ACTION_CMD), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_DISPLAY_CHEAT), false);
			::EnableWindow(GetDlgItem(IDC_WATCH_CHEAT), false);
			break;
		}
	}
	matchType = cMatchType.GetCurSel();
	if(cMode.GetCurSel() == 2) {
		cMatchType.AddString(_T("IP Range compare"));
		cMatchType.SetCurSel(2);
		cMatchType.EnableWindow(false);
	} else {
		if(cMatchType.GetCount() > 2)
			cMatchType.DeleteString(2);
		if(matchType > 1)
			cMatchType.SetCurSel(0);
		else
			cMatchType.SetCurSel(matchType);
		cMatchType.EnableWindow(true);
	}
}

/**
 * @file
 * $Id$
 */