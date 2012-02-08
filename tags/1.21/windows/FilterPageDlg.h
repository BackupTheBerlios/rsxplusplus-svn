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

#ifndef FILTERPAGE_DLG_H
#define FILTERPAGE_DLG_H

#include "WinUtil.h"
#include "../client/Util.h"

class FilterPageDlg : public CDialogImpl<FilterPageDlg> {
public:
	enum { IDD = IDD_ADD_FILTER_PAGE };

	BEGIN_MSG_MAP(FilterPageDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_FILTER_FONTCOLOR, onPickColor)
		COMMAND_ID_HANDLER(IDC_FILTER_BACKCOLOR, onPickColor)
		COMMAND_ID_HANDLER(IDC_FILTER_USECOLOR, onEnable)
		COMMAND_ID_HANDLER(IDC_FILTER_HAS_FONT_COLOR, onEnable)
		COMMAND_ID_HANDLER(IDC_FILTER_HAS_BG_COLOR, onEnable)
	END_MSG_MAP()

	FilterPageDlg() { 
		ctrlFstring.Detach();
		ctrlUseColor.Detach();
		ctrlBold.Detach();
		ctrlItalic.Detach();
		ctrlUnderline.Detach();
		ctrlStrikeout.Detach();
	};

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlFstring.SetFocus();
		return FALSE;
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT onPickColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT onEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	tstring fstring;
	bool useColor, hasFontColor, hasBgColor, bold, italic, underline, strikeout;
	COLORREF fontColor, backColor;

private:
	CEdit ctrlFstring;
	CButton ctrlHasFontColor;
	CButton ctrlHasBgColor;
	CButton ctrlUseColor;
	CButton ctrlBold;
	CButton ctrlItalic;
	CButton ctrlUnderline;
	CButton ctrlStrikeout;
	void fixControls();
};

#endif // !defined(FILTERPAGE_DLG_H)
