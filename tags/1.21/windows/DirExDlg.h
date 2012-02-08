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

#pragma once

#include <atlcrack.h>
#include "../client/Util.h"

class DirExDlg : public CDialogImpl<DirExDlg> {
	CEdit ctrlName;
	CEdit ctrlPath;
	CEdit ctrlExtensions;

public:
	DirExDlg() {};

	tstring name;
	tstring path;
	tstring extensions;

	enum { IDD = IDD_DIR_EX_DLG };

	BEGIN_MSG_MAP_EX(CommandDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_DIR_EX_BROWSE, OnBrowse);
	END_MSG_MAP()

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlName.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBrowse(UINT /*uMsg*/, WPARAM /*wParam*/, HWND /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {
			TCHAR buf[256];

			if((ctrlName.GetWindowTextLength() == 0) || (ctrlPath.GetWindowTextLength()== 0)){
				MessageBox(_T("Name and path must not be empty"));
				return 0;
			}

#define GET_TEXT(id, var) \
	GetDlgItemText(id, buf, 256); \
	var = buf;

			GET_TEXT(IDC_DIR_EX_NAME, name);
			GET_TEXT(IDC_DIR_EX_PATH, path);
			GET_TEXT(IDC_DIR_EX_EXTENSION, extensions);
			if(path[path.length() -1] != '\\')
				path += _T("\\");

		}
		EndDialog(wID);
		return 0;
	}
};