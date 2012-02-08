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

#ifndef RSXPLUSPLUS_KICK_DLG_H
#define RSXPLUSPLUS_KICK_DLG_H

#include "CRawCombo.h"

class RsxKickDlg : public CDialogImpl<RsxKickDlg> {
public:
	tstring rawCommand;
	tstring description;
	tstring title;

	enum { IDD = IDD_KICK_DLG };

	BEGIN_MSG_MAP(RsxKickDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_HANDLER(IDC_ACTION_LIST, CBN_SELCHANGE, onSelChange)
		COMMAND_HANDLER(IDC_LINE_RAW, CBN_SELCHANGE, onSelChange)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	~RsxKickDlg() {
		cRaw.Detach();
		cAction.Detach();
		ctrlCommand.Detach();
	};

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		cRaw.SetFocus();
		return FALSE;
	}
	LRESULT onSelChange(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
private:
	CComboBox cRaw;
	CRawCombo cAction;

	CStatic ctrlDescription;
	CEdit ctrlCommand;
};
#endif //RSXPLUSPLUS_KICK_DLG_H