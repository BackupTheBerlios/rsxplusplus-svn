#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "HLPageDlg.h"

#define ATTACH(id, var) var.Attach(GetDlgItem(id))
LRESULT HLPageDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	ATTACH(IDC_FSTRING,				ctrlFstring);
	ATTACH(IDC_FILTER_BOLD,			ctrlBold);
	ATTACH(IDC_FILTER_ITALIC,		ctrlItalic);
	ATTACH(IDC_FILTER_STRIKEOUT,	ctrlStrike);
	ATTACH(IDC_FILTER_UNDERLINE,	ctrlUnderline);
	ATTACH(IDC_HAS_BG_COLOR,		ctrlHasBgColor);
	ATTACH(IDC_HAS_FONT_COLOR,		ctrlHasFontColor);
	ATTACH(IDC_HL_DISPLAY_POPUP,	ctrlDisplayPopup);
	ATTACH(IDC_HL_FLASH_WINDOW,		ctrlFlashWindow);
	ATTACH(IDC_HL_PLAY_SOUND,		ctrlPlaySound);

	ctrlFstring.SetWindowText(fstring.c_str());
	ctrlBold.SetCheck(bold ? BST_CHECKED : BST_UNCHECKED);
	ctrlItalic.SetCheck(italic ? BST_CHECKED : BST_UNCHECKED);
	ctrlUnderline.SetCheck(underline ? BST_CHECKED : BST_UNCHECKED);
	ctrlStrike.SetCheck(strike ? BST_CHECKED : BST_UNCHECKED);
	ctrlHasBgColor.SetCheck(hasBgColor ? BST_CHECKED : BST_UNCHECKED);
	ctrlHasFontColor.SetCheck(hasFontColor ? BST_CHECKED : BST_UNCHECKED);
	ctrlDisplayPopup.SetCheck(displayPopup ? BST_CHECKED : BST_UNCHECKED);
	ctrlFlashWindow.SetCheck(flashWindow ? BST_CHECKED : BST_UNCHECKED);
	ctrlPlaySound.SetCheck(playSound ? BST_CHECKED : BST_UNCHECKED);

	CenterWindow(GetParent());
	fixControls();
	return FALSE;
}

LRESULT HLPageDlg::onPickColor(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	switch (wID) {
		case IDC_FILTER_FONTCOLOR: {
				CColorDialog colPicker(fontColor, CC_FULLOPEN, *this);
				if(colPicker.DoModal() == IDOK) {
					fontColor = colPicker.GetColor();
				}
			}
			break;
		case IDC_FILTER_BACKCOLOR: {
				CColorDialog colPicker(backColor, CC_FULLOPEN, *this);
				if(colPicker.DoModal() == IDOK) {
					backColor = colPicker.GetColor();
				}
			}
			break;
		default:
			break;
	}
	return true;
}

LRESULT HLPageDlg::onSelSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CFileDialog dlg(TRUE);
	dlg.m_ofn.lpstrFilter = _T("Wave Files\0*.wav\0\0");
	if(dlg.DoModal() == IDOK){
		soundFile = dlg.m_ofn.lpstrFile;
	}

	return 0;
}

LRESULT HLPageDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
#define GET_CHECK(x) x.GetCheck() == BST_CHECKED
	if(wID == IDOK) {
		TCHAR buf[512];
		if (ctrlFstring.GetWindowTextLength() == 0) {
			MessageBox(CTSTRING(LINE_EMPTY));
			return 0;
		}

		GetDlgItemText(IDC_FSTRING, buf, 512);
		fstring = buf;

		bold = GET_CHECK(ctrlBold);
		italic = GET_CHECK(ctrlItalic);
		underline = GET_CHECK(ctrlUnderline);
		strike = GET_CHECK(ctrlStrike);
		hasBgColor = GET_CHECK(ctrlHasBgColor);
		hasFontColor = GET_CHECK(ctrlHasFontColor);
		displayPopup = GET_CHECK(ctrlDisplayPopup);
		flashWindow = GET_CHECK(ctrlFlashWindow);
		playSound = GET_CHECK(ctrlPlaySound);
	}
#undef GET_CHECK
	EndDialog(wID);
	return 0;
}

LRESULT HLPageDlg::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

void HLPageDlg::fixControls() {
	BOOL use = IsDlgButtonChecked(IDC_HAS_BG_COLOR) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_FILTER_BACKCOLOR), use);
	use = IsDlgButtonChecked(IDC_HAS_FONT_COLOR) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_FILTER_FONTCOLOR), use);
	use = IsDlgButtonChecked(IDC_HL_PLAY_SOUND) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_SOUND_FILE_PATH), use);
}