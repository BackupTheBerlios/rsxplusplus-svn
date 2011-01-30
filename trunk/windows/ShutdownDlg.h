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

#ifndef RSXPLUSPLUS_SHUTDOWN_DLG
#define RSXPLUSPLUS_SHUTDOWN_DLG

class ShutdownDlg : public CDialogImpl<ShutdownDlg> {
public:
	enum { IDD = IDD_SHUTDOWN_DLG };

	BEGIN_MSG_MAP(ShutdownDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_HANDLER(IDC_TYPE, CBN_SELCHANGE, onSelChange)
	END_MSG_MAP()

	ShutdownDlg(HICON& i) : action(0), type(0), timeout(0), icon(i) { };

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSelChange(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */);

	int action;
	int type;
	uint64_t timeout;

private:
	HICON& icon;
	CComboBox ctrlAction;
	CComboBox ctrlType;
	CComboBox ctrlTimeType;
};

#endif

/**
 * @file
 * $Id$
 */
