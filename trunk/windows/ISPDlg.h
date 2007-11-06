#if !defined ISPDLG_H
#define ISPDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../client/Util.h"
#include "../client/FavoriteManager.h"

class ISPDlg : public CDialogImpl<ISPDlg>
{
	CEdit ctrlRange;
	CEdit ctrlISP;
	CButton ctrlBad;

public:
	
	string range;
	string ISP;
	bool bad;

	enum { IDD = IDD_ISP_DLG };

	BEGIN_MSG_MAP(ISPDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	ISPDlg() : bad(false) { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlRange.SetFocus();
		return FALSE;
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif