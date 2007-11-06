
#if !defined(AFX_FakeSharesDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)
#define AFX_FakeSharesDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FakeSharesDlg : public CDialogImpl<FakeSharesDlg>
{
	CEdit ctrlValue;
	CButton ctrlExact;
	CButton ctrlContains;

public:
	string value;
	string type;

	enum { IDD = IDD_ADD_FAKE_SHARES };

	BEGIN_MSG_MAP(FakeSharesDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, onFocus)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	FakeSharesDlg() { };

	LRESULT onFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		ctrlValue.SetFocus();
		return FALSE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{

#define ATTACH(id, var, txt) \
		var.Attach(GetDlgItem(id)); \
		var.SetWindowText(Text::toT(txt).c_str());

		ATTACH(IDC_VALUE, ctrlValue, value);
#undef ATTACH
#define ATTACH(id, var) var.Attach(GetDlgItem(id))
		ATTACH(IDC_EXACT, ctrlExact);
		ATTACH(IDC_CONTAINS, ctrlContains);
#undef ATTACH
		if ( type.compare(STRING(CONTAINS)) == 0 ) {
			ctrlContains.SetCheck(BST_CHECKED);
		} else {
			ctrlExact.SetCheck(BST_CHECKED);
		}

		ctrlValue.SetFocus();

		CenterWindow(GetParent());
		return FALSE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if(wID == IDOK) {

#define GET_TEXT(id, var) \
			GetDlgItemText(id, buf, 512); \
			var = Text::fromT(buf);
			TCHAR buf[512];
			//string temp;

			if ( ctrlValue.GetWindowTextLength() == 0 ) {
				MessageBox(_T("Value must not be empty"));
				return 0;
			}

			GET_TEXT(IDC_VALUE, value);
			if(ctrlExact.GetCheck() == BST_CHECKED) { 
				type = STRING(EXACT); 
			} else { 
				type = STRING(CONTAINS); 
			}
		}
		EndDialog(wID);
		return 0;
	}
};

#endif // !defined(AFX_FakeSharesDlg_H__A7EB85C3_1EEA_4FEC_8450_C090219B8619__INCLUDED_)

/**
 * @file
 * $Id: FakeSharesDlg.h,v 1.1 18:15 26/11/2004 Virus27 Exp $
 */
