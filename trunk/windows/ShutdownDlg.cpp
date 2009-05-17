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
#include "../client/DCPlusPlus.h"

#include "resource.h"
#include "ShutdownDlg.h"
#include "WinUtil.h"
#include "ShutdownManager.h"

LRESULT ShutdownDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ctrlAction.Attach(GetDlgItem(IDC_ACTION));
	ctrlType.Attach(GetDlgItem(IDC_TYPE));
	ctrlTimeType.Attach(GetDlgItem(IDC_TIME_TYPE));
	
	SetDlgItemInt(IDC_TIMEOUT, 0);

	ctrlAction.AddString(_T("Shutdown"));
	ctrlAction.AddString(_T("Logoff"));
	ctrlAction.AddString(_T("Reboot"));
	ctrlAction.AddString(_T("Suspend"));
	ctrlAction.AddString(_T("Hibernate"));
	ctrlAction.AddString(_T("Lock"));
	ctrlAction.AddString(_T("Close RSX++"));
	ctrlAction.SetCurSel(0);

	ctrlType.AddString(_T("All downloads are completed"));
	ctrlType.AddString(_T("All transfers are completed"));
	ctrlType.AddString(_T("Empty Queue"));
	ctrlType.AddString(_T("Time delay"));
	ctrlType.SetCurSel(1);

	ctrlTimeType.AddString(_T("Seconds"));
	ctrlTimeType.AddString(_T("Minutes"));
	ctrlTimeType.AddString(_T("Hours"));
	ctrlTimeType.SetCurSel(1);

	SetIcon(icon, FALSE);
	SetIcon(icon, TRUE);

	CenterWindow(GetParent());
	return FALSE;
}

LRESULT ShutdownDlg::onSelChange(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	if(ctrlType.GetCurSel() == ShutdownManager::DELAY) {
		::EnableWindow(GetDlgItem(IDC_TIMEOUT), 1);
		ctrlTimeType.EnableWindow(true);
	} else {
		::EnableWindow(GetDlgItem(IDC_TIMEOUT), 0);
		ctrlTimeType.EnableWindow(false);
	}
	return 0;
}

LRESULT ShutdownDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	action = ctrlAction.GetCurSel();
	type = ctrlType.GetCurSel();

	timeout = GetDlgItemInt(IDC_TIMEOUT);

	switch(ctrlTimeType.GetCurSel()) {
		case 1: timeout *= 60; break;
		case 2: timeout *= 60 * 60; break;
	}

	EndDialog(wID);
	return 0;
}

/**
 * @file
 * $Id: RawDlg.cpp 117 2009-04-11 21:44:24Z adrian_007 $
 */

