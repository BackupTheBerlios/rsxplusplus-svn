/*
 * Copyright (C) 2006-2008 Crise, crise@mail.berlios.de
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

#ifndef DETECTION_ENTRY_DLG
#define DETECTION_ENTRY_DLG

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/DetectionManager.h"
#include "../client/RawManager.h"
#include "../client/version.h"

#include "ExListViewCtrl.h"

class DetectionEntryDlg : public CDialogImpl<DetectionEntryDlg>
{
public:
	DetectionEntry& curEntry;

	enum { IDD = IDD_DETECTION_ENTRY };

	BEGIN_MSG_MAP(DetectionEntryDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDC_ADD, onAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, onRemove)
		COMMAND_ID_HANDLER(IDC_CHANGE, onChange)
		COMMAND_ID_HANDLER(IDC_ID_EDIT, onIdEdit)
		COMMAND_ID_HANDLER(IDC_NEXT, onNext)
		COMMAND_ID_HANDLER(IDC_BACK, onNext)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		NOTIFY_HANDLER(IDC_PARAMS, LVN_ITEMCHANGED, onItemchangedDirectories)
	END_MSG_MAP()

	DetectionEntryDlg(DetectionEntry& de) : curEntry(de), idChanged(false), origId(de.Id) { };

	~DetectionEntryDlg() {
		ctrlName.Detach(); ctrlComment.Detach(); ctrlLevel.Detach();
		ctrlCheat.Detach(); ctrlRaw.Detach(); ctrlParams.Detach();
	}

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlName.SetFocus();
		return FALSE;
	}

	LRESULT onIdEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		idChanged = true;
		::EnableWindow(GetDlgItem(IDC_DETECT_ID), true);
		::EnableWindow(GetDlgItem(IDC_ID_EDIT), false);
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onItemchangedDirectories(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CEdit ctrlName, ctrlComment, ctrlCheat;
	CRawCombo ctrlRaw;
	CComboBox ctrlLevel;
	ExListViewCtrl ctrlParams;
	bool idChanged;
	uint32_t origId;

	void updateVars();
	void updateControls();
};

#endif // DETECTION_ENTRY_DLG
