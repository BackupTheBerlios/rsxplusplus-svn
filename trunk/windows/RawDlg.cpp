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
#include "../client/ActionRaw.h"

#include "Resource.h"

#include "RawDlg.h"

LRESULT RawDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetDlgItemText(IDC_RAW_NAME, CTSTRING(NAME));

	ctrlName.Attach(GetDlgItem(IDC_NAME));
	ctrlRaw.Attach(GetDlgItem(IDC_RAW));
	ctrlTime.Attach(GetDlgItem(IDC_TIME));
	cVariables.Attach(GetDlgItem(IDC_RAW_VAR));
	cVar.Attach(GetDlgItem(IDC_RAW_VAR_EXAMPLE));
	ctrlLua.Attach(GetDlgItem(IDC_RAW_LUA));
	ctrlLua.SetCheck(useLua ? BST_CHECKED : BST_UNCHECKED);

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
	cVariables.AddString(_T("TestSUR String"));
	cVariables.AddString(_T("FileList Disconnects"));
	cVariables.AddString(_T("Timeouts"));
	cVariables.AddString(_T("XML Generator"));
	cVariables.AddString(_T("ADLSearch FileName"));
	cVariables.AddString(_T("ADLSearch Comment"));
	cVariables.AddString(_T("ADLSearch FileSize"));
	cVariables.AddString(_T("ADLSearch TTH"));
	cVariables.AddString(_T("ADLSearch Forbidden Size"));
	cVariables.AddString(_T("ADLSearch Total Points"));
	cVariables.AddString(_T("ADLSearch Files Count"));

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
		case 4: cVar.SetWindowText(_T("%[userCL]")); break;
		case 5: cVar.SetWindowText(_T("%[userMT]")); break;
		case 6: cVar.SetWindowText(_T("%[userCS]")); break;
		case 7: cVar.SetWindowText(_T("%[userTA]")); break;
		case 8: cVar.SetWindowText(_T("%[userPK]")); break;
		case 9: cVar.SetWindowText(_T("%[userLO]")); break;
		case 10: cVar.SetWindowText(_T("%[userSU]")); break;
		case 11: cVar.SetWindowText(_T("%[userCM]")); break;
		case 12: cVar.SetWindowText(_T("%[userIS]")); break;
		case 13: cVar.SetWindowText(_T("%[userTS]")); break;
		case 14: cVar.SetWindowText(_T("%[userFD]")); break;
		case 15: cVar.SetWindowText(_T("%[userTO]")); break;
		case 16: cVar.SetWindowText(_T("%[userFG]")); break;
		case 17: cVar.SetWindowText(_T("%[userA1]")); break;
		case 18: cVar.SetWindowText(_T("%[userA2]")); break;
		case 19: cVar.SetWindowText(_T("%[userA3]")); break;
		case 20: cVar.SetWindowText(_T("%[userA4]")); break;
		case 21: cVar.SetWindowText(_T("%[userA5]")); break;
		case 22: cVar.SetWindowText(_T("%[userA6]")); break;
		case 23: cVar.SetWindowText(_T("%[userA7]")); break;

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
		useLua = ctrlLua.GetCheck() == BST_CHECKED;
	}
	EndDialog(wID);
	return 0;
}

/**
 * @file
 * $Id$
 */

