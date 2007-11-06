#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "FilterPageDlg.h"

#define ATTACH(id, var) var.Attach(GetDlgItem(id))
LRESULT FilterPageDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	ATTACH(IDC_FSTRING,			ctrlFstring);
	ATTACH(IDC_FILTER_USECOLOR, ctrlUseColor);
	ATTACH(IDC_FILTER_BOLD,		ctrlBold);
	ATTACH(IDC_FILTER_ITALIC,	ctrlItalic);
	ATTACH(IDC_FILTER_UNDERLINE,ctrlUnderline);
	ATTACH(IDC_FILTER_STRIKEOUT,ctrlStrikeout);
	ATTACH(IDC_FILTER_HAS_FONT_COLOR, ctrlHasFontColor);
	ATTACH(IDC_FILTER_HAS_BG_COLOR, ctrlHasBgColor);

	ctrlFstring.SetWindowText(fstring.c_str());
	ctrlUseColor.SetCheck(useColor ?	BST_CHECKED : BST_UNCHECKED);
	ctrlBold.SetCheck(bold ?			BST_CHECKED : BST_UNCHECKED);
	ctrlItalic.SetCheck(italic ?		BST_CHECKED : BST_UNCHECKED);
	ctrlUnderline.SetCheck(underline ?	BST_CHECKED : BST_UNCHECKED);
	ctrlStrikeout.SetCheck(strikeout ?	BST_CHECKED : BST_UNCHECKED);
	ctrlHasFontColor.SetCheck(hasFontColor ? BST_CHECKED : BST_UNCHECKED);
	ctrlHasBgColor.SetCheck(hasBgColor ? BST_CHECKED : BST_UNCHECKED);

	CenterWindow(GetParent());
	fixControls();
	return FALSE;
}

LRESULT FilterPageDlg::onPickColor(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
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

LRESULT FilterPageDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if(wID == IDOK) {
		TCHAR buf[512];
		if (ctrlFstring.GetWindowTextLength() == 0) {
			MessageBox(CTSTRING(LINE_EMPTY));
			return 0;
		}

		GetDlgItemText(IDC_FSTRING, buf, 512);
		fstring = buf;
		useColor = ctrlUseColor.GetCheck() == BST_CHECKED;
		hasFontColor = ctrlHasFontColor.GetCheck() == BST_CHECKED;
		hasBgColor = ctrlHasBgColor.GetCheck() == BST_CHECKED;
		bold = ctrlBold.GetCheck() == BST_CHECKED;
		italic = ctrlItalic.GetCheck() == BST_CHECKED;
		underline = ctrlUnderline.GetCheck() == BST_CHECKED;
		strikeout = ctrlStrikeout.GetCheck() == BST_CHECKED;
	}
	EndDialog(wID);
	return 0;
}

LRESULT FilterPageDlg::onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	fixControls();
	return 0;
}

void FilterPageDlg::fixControls() {
	BOOL use = IsDlgButtonChecked(IDC_FILTER_USECOLOR) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_FILTER_BOLD),				use);
	::EnableWindow(GetDlgItem(IDC_FILTER_ITALIC),			use);
	::EnableWindow(GetDlgItem(IDC_FILTER_UNDERLINE),		use);
	::EnableWindow(GetDlgItem(IDC_FILTER_STRIKEOUT),		use);
	::EnableWindow(GetDlgItem(IDC_FILTER_HAS_FONT_COLOR),	use);
	::EnableWindow(GetDlgItem(IDC_FILTER_HAS_BG_COLOR),		use);

	BOOL appendColor = IsDlgButtonChecked(IDC_FILTER_HAS_FONT_COLOR) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_FILTER_FONTCOLOR), use && appendColor);
	appendColor = IsDlgButtonChecked(IDC_FILTER_HAS_BG_COLOR) == BST_CHECKED;
	::EnableWindow(GetDlgItem(IDC_FILTER_BACKCOLOR), use && appendColor);
}