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

#if !defined(HL_PAGE_DLG_H)
#define HL_PAGE_DLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinUtil.h"
#include "../client/Util.h"

class HLPageDlg : public CDialogImpl<HLPageDlg> {
	CEdit ctrlFstring;
	CButton ctrlHasFontColor, ctrlHasBgColor, ctrlBold, ctrlItalic, ctrlUnderline, ctrlStrike;
	CButton ctrlDisplayPopup, ctrlPlaySound, ctrlFlashWindow;
public:
	tstring fstring, soundFile;
	bool hasFontColor, hasBgColor, bold, italic, underline, strike;
	bool playSound, displayPopup, flashWindow;

	enum { IDD = IDD_ADD_HL };

	BEGIN_MSG_MAP(HLPageDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_FILTER_FONTCOLOR, onPickColor)
		COMMAND_ID_HANDLER(IDC_FILTER_BACKCOLOR, onPickColor)
		COMMAND_ID_HANDLER(IDC_SOUND_FILE_PATH, onSelSound)
		COMMAND_ID_HANDLER(IDC_HAS_BG_COLOR, onEnable)
		COMMAND_ID_HANDLER(IDC_HAS_FONT_COLOR, onEnable)
		COMMAND_ID_HANDLER(IDC_HL_PLAY_SOUND, onEnable)
	END_MSG_MAP()

	HLPageDlg() { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlFstring.SetFocus();
		return FALSE;
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onPickColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onSelSound(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	COLORREF fontColor, backColor;

private:
	void fixControls();
};

#endif // !defined(FILTERPAGE_DLG_H)
