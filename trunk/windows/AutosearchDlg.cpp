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
#include "Resource.h"
#include "WinUtil.h"
#include "AutosearchDlg.h"

#define GET_TEXT(id, var) \
	GetDlgItemText(id, buf, 1024); \
	var = Text::fromT(buf);

#define ATTACH(id, var) var.Attach(GetDlgItem(id))

AutosearchPageDlg::AutosearchPageDlg() {
	search = _T("");
	fileType = 0;
	action = 0;
	rawToSend = 0;
	display = false;
	comment = _T("");
}

AutosearchPageDlg::~AutosearchPageDlg() {
	ctrlSearch.Detach();
	ctrlCheatingDescription.Detach();
	ctrlFileType.Detach();
	cRaw.Detach();
	cDisplay.Detach();
	cAction.Detach();
	ftImage.Destroy();
}

LRESULT AutosearchPageDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	ATTACH(IDC_AS_SEARCH_STRING, ctrlSearch);
	ATTACH(IDC_AS_CHEAT, ctrlCheatingDescription);

	ctrlSearch.SetWindowText(search.c_str());
	ctrlCheatingDescription.SetWindowText(comment.c_str());

	ATTACH(IDC_AS_FILETYPE, ctrlFileType);

	ResourceLoader::LoadImageList(IDP_SEARCH_TYPES, ftImage, 16, 16);
	ctrlFileType.SetImageList(ftImage);

	int q = 0;
	for(size_t i = 0; i < 10; i++) {
		COMBOBOXEXITEM cbitem = {CBEIF_TEXT|CBEIF_IMAGE|CBEIF_SELECTEDIMAGE};
		tstring ftString;
		switch(i) {
			case 0: q = 0; ftString = TSTRING(ANY); break;
			case 1: q = 1; ftString = TSTRING(AUDIO); break;
			case 2: q = 2; ftString = TSTRING(COMPRESSED); break;
			case 3: q = 3; ftString = TSTRING(DOCUMENT); break;
			case 4: q = 4; ftString = TSTRING(EXECUTABLE); break;
			case 5: q = 5; ftString = TSTRING(PICTURE); break;
			case 6: q = 6; ftString = TSTRING(VIDEO); break;
			case 7: q = 7; ftString = TSTRING(DIRECTORY); break;
			case 8: q = 8; ftString = _T("TTH"); break;
			case 9: q = 0; ftString = _T("RegExp"); break;
		}
		cbitem.pszText = const_cast<TCHAR*>(ftString.c_str());
		cbitem.iItem = i; 
		cbitem.iImage = q;
		cbitem.iSelectedImage = q;
		ctrlFileType.InsertItem(&cbitem);
	}
	ctrlFileType.SetCurSel(fileType);

	cRaw.attach(GetDlgItem(IDC_AS_RAW), rawToSend);

	ATTACH(IDC_AS_DISPLAY, cDisplay);
	cDisplay.SetCheck(display ?	BST_CHECKED : BST_UNCHECKED);

	ATTACH(IDC_AS_ACTION, cAction);
	cAction.AddString(_T("Send Raw"));
	cAction.AddString(_T("Download"));
	cAction.AddString(_T("Add to Queue"));
	cAction.AddString(_T("Check user File List"));
	cAction.SetCurSel(action);

	fixControls();
	CenterWindow(GetParent());
	return TRUE;
}

LRESULT AutosearchPageDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		TCHAR buf[512];
		if (ctrlSearch.GetWindowTextLength() == 0) {
			MessageBox(CTSTRING(LINE_EMPTY));
			return 0;
		}
		GetDlgItemText(IDC_AS_SEARCH_STRING, buf, 512);
		search = buf;
		fileType = ctrlFileType.GetCurSel();
		action = cAction.GetCurSel();
		rawToSend = cRaw.getActionId();
		display = cDisplay.GetCheck() == BST_CHECKED;
		GetDlgItemText(IDC_AS_CHEAT, buf, 512);
		comment = buf;
	}
	EndDialog(wID);
	return 0;
}
LRESULT AutosearchPageDlg::onAction(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

void AutosearchPageDlg::fixControls() {
	switch(cAction.GetCurSel()) {
		case 0: 
			{ //ban
				::EnableWindow(GetDlgItem(IDC_AS_CHEAT), true);
				::EnableWindow(GetDlgItem(IDC_AS_DISPLAY), true);
				::EnableWindow(GetDlgItem(IDC_AS_RAW), true);
				break; 
			}
		case 1: 
			{ //download
				::EnableWindow(GetDlgItem(IDC_AS_CHEAT), false);
				::EnableWindow(GetDlgItem(IDC_AS_DISPLAY), false);
				::EnableWindow(GetDlgItem(IDC_AS_RAW), false);
				break; 
			}
		case 2: 
			{ //add to queue
				::EnableWindow(GetDlgItem(IDC_AS_CHEAT), false);
				::EnableWindow(GetDlgItem(IDC_AS_DISPLAY), false);
				::EnableWindow(GetDlgItem(IDC_AS_RAW), false);
				break; 
			}
		case 3: 
			{ //check file list
				::EnableWindow(GetDlgItem(IDC_AS_CHEAT), false);
				::EnableWindow(GetDlgItem(IDC_AS_DISPLAY), false);
				::EnableWindow(GetDlgItem(IDC_AS_RAW), false);
				break; 
			}
	}
}