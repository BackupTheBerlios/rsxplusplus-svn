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

#ifndef RSXPLUSPLUS_AUTOSEARCH_DLG
#define RSXPLUSPLUS_AUTOSEARCH_DLG

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <atlcrack.h>
#include "ResourceLoader.h"

#include "../client/Util.h"
#include "../client/ResourceManager.h"
#include "CRawCombo.h"

class AutosearchPageDlg : public CDialogImpl<AutosearchPageDlg> {
public:
	tstring	search, comment;
	int fileType, action, rawToSend;
	bool display;

	enum { IDD = IDD_AUTOSEARCH_DLG };

	AutosearchPageDlg();
	~AutosearchPageDlg();

	BEGIN_MSG_MAP(AutosearchPageDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_AS_ACTION, onAction)
	END_MSG_MAP()


	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlSearch.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	void fixControls();
	CImageList ftImage;

	CEdit ctrlSearch, ctrlCheatingDescription;
	CRawCombo cRaw;
	CComboBoxEx ctrlFileType;
	CComboBox cAction;
	CButton cDisplay;
};
#endif // RSXPLUSPLUS_AUTOSEARCH_DLG
