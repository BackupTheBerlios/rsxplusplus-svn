/* 
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

#if !defined(FILTERPAGE_DLG_H)
#define FILTERPAGE_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/RawManager.h"

class IPWatchDlg : public CDialogImpl<IPWatchDlg>, protected RawSelector {
	CEdit cIP;
	CEdit cCheat;
	CComboBox cAction;
	CComboBox cActionCommand;
	CButton cDisplay, cRegExp;
public:
	tstring ip, cheat;
	int action, actionCmd;
	bool display, regexp;

	enum { IDD = IDD_ADD_IPWATCH };

	BEGIN_MSG_MAP(FilterPageDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_WATCH_ACTION, onAction)
	END_MSG_MAP()

	IPWatchDlg() { };
	~IPWatchDlg() {
		cActionCommand.Detach();
		cAction.Detach();
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		cIP.SetFocus();
		return FALSE;
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	void fixControls();
};

#endif // !defined(FILTERPAGE_DLG_H)
