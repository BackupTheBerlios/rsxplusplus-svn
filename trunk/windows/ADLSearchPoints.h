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

#ifndef RSXPLUSPLUS_ADLSEARCH_POINTS_VALUE_H
#define RSXPLUSPLUS_ADLSEARCH_POINTS_VALUE_H

#include "CRawCombo.h"

class PointValue : public CDialogImpl<PointValue> {
public:
	enum { IDD = IDD_ADD_ADLSP_RANGE };

	int value;
	int actionId;

	BEGIN_MSG_MAP(PointValue)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	PointValue() : actionId(0), value(0) { };

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlValue.Attach(GetDlgItem(IDC_VALUE));
		ctrlValue.SetWindowText(Util::toStringW(value).c_str());
		ctrlAction.attach(GetDlgItem(IDC_ACTION), actionId);
		ctrlValue.SetFocus();
		CenterWindow(GetParent());
		return FALSE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if(wID == IDOK) {
			if(ctrlValue.GetWindowTextLength() == 0) {
				MessageBox(_T("Value must not be empty"));
				return 0;
			}
			value = GetDlgItemInt(IDC_VALUE);
			if(value <= 0) {
				MessageBox(_T("Value must be greater than 0"));
				return 0;
			}
			actionId = ctrlAction.getActionId();
		}
		EndDialog(wID);
		return 0;
	}
private:
	CEdit ctrlValue;
	CRawCombo ctrlAction;
};
#endif