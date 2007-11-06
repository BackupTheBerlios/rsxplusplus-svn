/* 
 * Copyright (C) 2005-2005 Virus27, Virus27@free.fr
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

#include "RawDlg.h"

LRESULT RawDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetDlgItemText(IDC_RAW_NAME, CTSTRING(NAME));

	ctrlName.Attach(GetDlgItem(IDC_NAME));
	ctrlRaw.Attach(GetDlgItem(IDC_RAW));
	ctrlTime.Attach(GetDlgItem(IDC_TIME));
	cVariables.Attach(GetDlgItem(IDC_RAW_VAR));
	cVar.Attach(GetDlgItem(IDC_RAW_VAR_EXAMPLE));

	ctrlName.SetWindowText(Text::toT(name).c_str());
	ctrlRaw.SetWindowText(Text::toT(raw).c_str());
	ctrlTime.SetWindowText(Text::toT(Util::toString(time)).c_str());

	cVariables.AddString(_T("My Nick"));
	cVariables.AddString(_T("User Nick"));
	cVariables.AddString(_T("IP"));
	cVariables.AddString(_T("Host"));
	cVariables.AddString(_T("Client Type"));
	cVariables.AddString(_T("MyINFO Type"));
	cVariables.AddString(_T("Cheat"));
	cVariables.AddString(_T("Tag"));
	cVariables.AddString(_T("PK"));
	cVariables.AddString(_T("Lock"));
	cVariables.AddString(_T("Supports"));
	cVariables.AddString(_T("Comment"));
	cVariables.AddString(_T("ISP"));

	cVariables.SetCurSel(0);
	cVar.SetWindowText(_T("%[myNI]"));

	CenterWindow(GetParent());
	return FALSE;
}

LRESULT RawDlg::onSelChange(WORD /* wNotifyCode */, WORD /*wID*/, HWND /* hWndCtl */, BOOL& /* bHandled */) {
	switch(cVariables.GetCurSel()) {
		case 0: cVar.SetWindowText(_T("%[myNI]")); break;
		case 1: cVar.SetWindowText(_T("%[userNI]")); break;
		case 2: cVar.SetWindowText(_T("%[userI4]")); break;
		case 3: cVar.SetWindowText(_T("%[userHT]")); break;
		case 4: cVar.SetWindowText(_T("%[userCT]")); break;
		case 5: cVar.SetWindowText(_T("%[userMT]")); break;
		case 6: cVar.SetWindowText(_T("%[userCS]")); break;
		case 7: cVar.SetWindowText(_T("%[userTA]")); break;
		case 8: cVar.SetWindowText(_T("%[userPK]")); break;
		case 9: cVar.SetWindowText(_T("%[userLO]")); break;
		case 10: cVar.SetWindowText(_T("%[userSU]")); break;
		case 11: cVar.SetWindowText(_T("%[userCM]")); break;
		case 12: cVar.SetWindowText(_T("%[userIS]")); break;
	}
	return 0;
}

LRESULT RawDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		TCHAR buf[1024];
		GetDlgItemText(IDC_NAME, buf, 256);
		name = Text::fromT(buf);
		GetDlgItemText(IDC_RAW, buf, 1024);
		raw = Text::fromT(buf);
		GetDlgItemText(IDC_TIME, buf, 256);
		time = Util::toInt(Text::fromT(buf));
	}
	EndDialog(wID);
	return 0;
}

/**
 * @file
 * $Id: RawDlg.cpp 32 2007-09-16 20:49:44Z adrian_007 $
 */

